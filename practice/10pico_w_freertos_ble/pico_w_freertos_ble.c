#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "btstack_tlv.h"
#include "btstack_config.h"
#include "btstack.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/* BEGIN FREERTOS EVENTGROUP BITS */
#define mainLED_ON_BIT      ( 1UL << 0UL )
#define mainLED_OFF_BIT     ( 1UL << 1UL )
#define mainLED_BLINK_BIT   ( 1UL << 2UL )
/* END   FREERTOS EVENTGROUP BITS */

#define TLV_TAG_HOGD ((((uint32_t) 'H') << 24 ) | (((uint32_t) 'O') << 16) | (((uint32_t) 'G') << 8) | 'D')

/* BEGIN FREERTOS HANDLES */
static EventGroupHandle_t xEventGroup;
static TaskHandle_t xSettingHandle;
static TaskHandle_t xReadingHandle;
/* END   FREERTOS HANDLES */

static enum
{
    W4_WORKING,
    W4_HID_DEVICE_FOUND,
    W4_CONNECTED,
    W4_ENCRYPTED,
    W4_HID_SERVICE_FOUND,
    W4_HID_CHARACTERISTICS_FOUND,
    W4_BOOT_KEYBOARD_ENABLED,
    W4_BOOT_MOUSE_ENABLED,
    READY,
    W4_TIMEOUT_THEN_SCAN,
    W4_TIMEOUT_THEN_RECONNECT,
} app_state;

static le_device_addr_t remote_device;
static hci_con_handle_t connection_handle;

// used for GATT queries
static gatt_client_service_t       hid_service;
static gatt_client_characteristic_t protocol_mode_characteristic;
static gatt_client_characteristic_t boot_keyboard_input_characteristic;
static gatt_client_characteristic_t boot_mouse_input_characteristic;
static gatt_client_notification_t   keyboard_notifications;
static gatt_client_notification_t   mouse_notifications;

// used to implement connection timeout and reconnect timer
static btstack_timer_source_t connection_timer;

// register for events from HCI/GAP and SM
static btstack_packet_callback_registration_t hci_event_callback_registration;
static btstack_packet_callback_registration_t sm_event_callback_registration;

// used to store remote device in TLV
static const btstack_tlv_t * btstack_tlv_singleton_impl;
static void *                btstack_tlv_singleton_context;

static uint8_t hid_descriptor_storage[500];

/* I know, this isn't managed well in terms of dir structure...
   Next step is to learn what is the standard way of managing things
   in regards to FreeRTOS */

/* BEGIN LED TASK DEFINITIONS */

/* most likely won't use this function for real use case */
static void vEventBitSettingTask( void * pvParameters )
{
    const TickType_t xDelay = pdMS_TO_TICKS( 1000UL ), xDontBlock = 0;

    for( ;; )
    {
        // set bit to turn on 
        vTaskDelay( xDelay );
        xEventGroupSetBits( xEventGroup, mainLED_ON_BIT );
        vTaskDelay( xDelay );

        // set bit to turn off
        xEventGroupSetBits( xEventGroup, mainLED_OFF_BIT );
        vTaskDelay( xDelay );

        // set bit to blink
        xEventGroupSetBits( xEventGroup, mainLED_BLINK_BIT );
        vTaskDelay( pdMS_TO_TICKS( 3000UL ) );
    }
}

static void vEventBitReadingTask( void * pvParameters )
{
    EventBits_t xEventGroupValue;
    const EventBits_t xBitsToWaitFor = ( mainLED_ON_BIT | mainLED_OFF_BIT | mainLED_BLINK_BIT );

    // init stuff 
    if( cyw43_arch_init() )
    {
        // failed something...
    }

    for( ;; )
    {
        xEventGroupValue = xEventGroupWaitBits( xEventGroup, xBitsToWaitFor, pdTRUE, pdFALSE, portMAX_DELAY );

        if( ( xEventGroupValue & mainLED_ON_BIT ) != 0 )
        {
            cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 );
            vTaskDelay( pdMS_TO_TICKS( 1000UL ) );
        }
        else if( ( xEventGroupValue & mainLED_OFF_BIT ) != 0 )
        {
            cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
            vTaskDelay( pdMS_TO_TICKS( 1000UL ) );
        }
        else if( ( xEventGroupValue & mainLED_BLINK_BIT ) != 0 )
        {
            for( ;; )
            {
                cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 );
                vTaskDelay( pdMS_TO_TICKS( 100 ) );
                cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
                vTaskDelay( pdMS_TO_TICKS( 100 ) );
                cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 );
                vTaskDelay( pdMS_TO_TICKS( 100 ) );
                cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
                vTaskDelay( pdMS_TO_TICKS( 100 ) );
                cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 );
                vTaskDelay( pdMS_TO_TICKS( 100 ) );
                cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
                vTaskDelay( pdMS_TO_TICKS( 500 ) );

                /* Check if there is an incoming event notification, DON'T CLEAR it and exit immediately */
                xEventGroupValue = xEventGroupWaitBits( xEventGroup, xBitsToWaitFor, pdFALSE, pdFALSE, 0 );

                if( ( xEventGroupValue & mainLED_ON_BIT ) != 0 )
                {
                    break;
                }

                if( ( xEventGroupValue & mainLED_OFF_BIT ) != 0 )
                {
                    break;
                }
            }
        }
    }
}
/* END LED TASK DEFINITIONS */

static void hog_start_scan(void){
    printf("Scanning for LE HID devices...\n");
    app_state = W4_HID_DEVICE_FOUND;
    // Passive scanning, 100% (scan interval = scan window)
    gap_set_scan_parameters(0,48,48);
    gap_start_scan();
}

static void hog_connect(void) {
    // set timer
    btstack_run_loop_set_timer(&connection_timer, 10000);
    btstack_run_loop_set_timer_handler(&connection_timer, &hog_connection_timeout);
    btstack_run_loop_add_timer(&connection_timer);
    app_state = W4_CONNECTED;
    gap_connect(remote_device.addr, remote_device.addr_type);
}

static void hog_start_connect( void )
{
    // check if we have a bonded device
    btstack_tlv_get_instance( &btstack_tlv_singleton_impl, &btstack_tlv_singleton_context );
    if (btstack_tlv_singleton_impl){
        int len = btstack_tlv_singleton_impl->get_tag(btstack_tlv_singleton_context, TLV_TAG_HOGD, (uint8_t *) &remote_device, sizeof(remote_device));
        if (len == sizeof(remote_device)){
            printf("Bonded, connect to device with %s address %s ...\n", remote_device.addr_type == 0 ? "public" : "random" , bd_addr_to_str(remote_device.addr));
            hog_connect();
            return;
        }
    }
    // otherwise, scan for HID devices
    hog_start_scan();
}

static void packet_handler( uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t size )
{
    /* LISTING_PAUSE */
    UNUSED(channel);
    UNUSED(size);
    uint8_t   event;
    /* LISTING_RESUME */
    switch (packet_type) {
        case HCI_EVENT_PACKET:
            event = hci_event_packet_get_type(packet);
            switch (event) {
                case BTSTACK_EVENT_STATE:
                    if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) break;
                    btstack_assert(app_state == W4_WORKING);
                    hog_start_connect();
                    break;
                case GAP_EVENT_ADVERTISING_REPORT:
                    if (app_state != W4_HID_DEVICE_FOUND) break;
                    if (adv_event_contains_hid_service(packet) == false) break;
                    // stop scan
                    gap_stop_scan();
                    // store remote device address and type
                    gap_event_advertising_report_get_address(packet, remote_device.addr);
                    remote_device.addr_type = gap_event_advertising_report_get_address_type(packet);
                    // connect
                    printf("Found, connect to device with %s address %s ...\n", remote_device.addr_type == 0 ? "public" : "random" , bd_addr_to_str(remote_device.addr));
                    hog_connect();
                    break;
                case HCI_EVENT_DISCONNECTION_COMPLETE:
                    if (app_state != READY) break;
                    
                    connection_handle = HCI_CON_HANDLE_INVALID;
                    switch (app_state){
                        case READY:
                            printf("\nDisconnected, try to reconnect...\n");
                            app_state = W4_TIMEOUT_THEN_RECONNECT;
                            break;
                        default:
                            printf("\nDisconnected, start over...\n");
                            app_state = W4_TIMEOUT_THEN_SCAN;
                            break;
                    }
                    // set timer
                    btstack_run_loop_set_timer(&connection_timer, 100);
                    btstack_run_loop_set_timer_handler(&connection_timer, &hog_reconnect_timeout);
                    btstack_run_loop_add_timer(&connection_timer);
                    break;
                case HCI_EVENT_META_GAP:
                    // wait for connection complete
                    if (hci_event_gap_meta_get_subevent_code(packet) != GAP_SUBEVENT_LE_CONNECTION_COMPLETE) break;
                    if (app_state != W4_CONNECTED) return;
                    btstack_run_loop_remove_timer(&connection_timer);
                    connection_handle = gap_subevent_le_connection_complete_get_connection_handle(packet);
                    // request security
                    app_state = W4_ENCRYPTED;
                    sm_request_pairing(connection_handle);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

/* BEGIN BLE DEFINITIONS */
/**
 * @brief MUST BE CALLED WITHIN A FREERTOS TASK!!! 
 */
void BLE_server_init( void )
{
    l2cap_init();
    sm_init();
    gatt_client_init();
    att_server_init();
    hids_client_init( hid_descriptor_storage, sizeof( hid_descriptor_storage ) );

    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler( &hci_event_callback_registration );

    sm_event_callback_registration.callback = &sm_packet_handler;
    sm_add_event_handler( &sm_event_callback_registration );

    sm_set_io_capabilities( IO_CAPABILITY_DISPLAY_ONLY );
    sm_set_authentication_requirements( SM_AUTHREQ_BONDING );

    app_state = W4_WORKING;

    hci_power_control( HCI_POWER_ON );
}
/* END BLE DEFINITIONS */

int main()
{
    stdio_init_all();

    BaseType_t xStatus;
    xEventGroup = xEventGroupCreate();

    xStatus = xTaskCreate( vEventBitSettingTask, "SETTING", 1024, NULL, configMAX_PRIORITIES - 4, &xSettingHandle );
    xStatus = xTaskCreate( vEventBitReadingTask , "READ", 1024, NULL, configMAX_PRIORITIES - 3, &xReadingHandle );

    if( xStatus != pdPASS )
    {
        printf("Error: xTaskCreate failed");
    }
    else
    {
        vTaskStartScheduler();
    }

    for( ;; );

    return 0;
}
