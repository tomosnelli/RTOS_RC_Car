#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define mainLED_ON_BIT      ( 1UL << 0UL )
#define mainLED_OFF_BIT     ( 1UL << 1UL )
#define mainLED_BLINK_BIT   ( 1UL << 2UL )

static EventGroupHandle_t xEventGroup;
static TaskHandle_t SettingHandle;
static TaskHandle_t ReadingHandle;

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

/* BEGIN BLE DEFINITIONS */
void BLE_server_init( void )
{
    l2cap_init();

    sm_init();

    gatt_client_init();

    att_server_init();

    hids_client_init();

    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler( &hci_event_callback_registration );

    sm_event_callback_registration.callback = &sm_packet_handler;
    sm_add_event_handler( &sm_event_callback_registration );

    sm_event_callback_registration.callback = &sm_packet_handler;
    sm_add_event_handler( &sm_event_callback_registration );

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

    xStatus = xTaskCreate( vEventBitSettingTask, "SETTING", 1024, NULL, configMAX_PRIORITIES - 4, &SettingHandle );
    xStatus = xTaskCreate( vEventBitReadingTask , "READ", 1024, NULL, configMAX_PRIORITIES - 3, &ReadingHandle );

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
