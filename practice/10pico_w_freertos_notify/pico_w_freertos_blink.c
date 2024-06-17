#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

// #define mainLED_ON_BIT      ( 1UL << 0UL )
// #define mainLED_OFF_BIT     ( 1UL << 1UL )
// #define mainLED_BLINK_BIT   ( 1UL << 2UL )
// 
// static EventGroupHandle_t xEventGroup;
// static TaskHandle_t SettingHandle;
// static TaskHandle_t ReadingHandle;

static TaskHandle_t xNotifyHandle;
static TaskHandle_t xWaitHandle;

#define mainLED_ON    0x00
#define mainLED_OFF   0x01
#define mainLED_BLINK 0x02

/**
 * PICO blink control with FreeRTOS event groups.
 * Previously tried queues but kinda got messy
 */

// static void vEventBitSettingTask( void * pvParameters )
// {
//     const TickType_t xDelay = pdMS_TO_TICKS( 1000UL ), xDontBlock = 0;
// 
//     for( ;; )
//     {
//         vTaskDelay( xDelay );
// 
//         // set bit to turn on 
//         xEventGroupSetBits( xEventGroup, mainLED_ON_BIT );
//         vTaskDelay( xDelay );
// 
//         // set bit to turn off
//         xEventGroupSetBits( xEventGroup, mainLED_OFF_BIT );
//         vTaskDelay( xDelay );
// 
//         // set bit to blink
//         xEventGroupSetBits( xEventGroup, mainLED_BLINK_BIT );
//         vTaskDelay( pdMS_TO_TICKS( 3000UL ) );
//     }
// }

static void vTaskSendNotification( void * pvParameters )
{

    const TickType_t xDelay = pdMS_TO_TICKS( 1000UL ), xMoreDelay = pdMS_TO_TICKS( 5000UL );

    for ( ;; )
    {
        xTaskNotify( xWaitHandle, mainLED_ON, eSetValueWithOverwrite );
        vTaskDelay( xDelay );

        xTaskNotify( xWaitHandle, mainLED_OFF, eSetValueWithOverwrite );
        vTaskDelay( xDelay );

        xTaskNotify( xWaitHandle, mainLED_BLINK, eSetValueWithOverwrite );
        vTaskDelay( xMoreDelay );
    }
}

static void vTaskReceiveNotification( void * pvParameters )
{
    // EventBits_t xEventGroupValue;
    // const EventBits_t xBitsToWaitFor = ( mainLED_ON_BIT | mainLED_OFF_BIT | mainLED_BLINK_BIT );
    uint32_t ulNotifiedValue;

    // init stuff 
    if( cyw43_arch_init() )
    {
        // failed something...
        for( ;; );
    }

    for( ;; )
    {
        // xEventGroupValue = xEventGroupWaitBits( xEventGroup, xBitsToWaitFor, pdTRUE, pdFALSE, portMAX_DELAY );
        xTaskNotifyWait( 0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY );

        if( ( ulNotifiedValue & mainLED_ON ) != 0 )
        {
            cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 );
            vTaskDelay( pdMS_TO_TICKS( 1000UL ) );
        }
        else if( ( ulNotifiedValue & mainLED_OFF ) != 0 )
        {
            cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
            vTaskDelay( pdMS_TO_TICKS( 1000UL ) );
        }
        else if( ( ulNotifiedValue & mainLED_BLINK ) != 0 )
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
                xTaskNotifyWait( 0x00, 0x00, &ulNotifiedValue, 0 );

                if( ( ulNotifiedValue & mainLED_ON ) != 0 )
                {
                    break;
                }

                if( ( ulNotifiedValue & mainLED_OFF ) != 0 )
                {
                    break;
                }
            }
        }
    }
}

int main()
{
    stdio_init_all();

    BaseType_t xStatus = pdPASS;
    // xEventGroup = xEventGroupCreate();

    xStatus &= xTaskCreate( vTaskSendNotification, "Notify", 1024, NULL, configMAX_PRIORITIES - 4, &xNotifyHandle );
    xStatus &= xTaskCreate( vTaskReceiveNotification, "WaitNotify", 1024, NULL, configMAX_PRIORITIES - 3, &xWaitHandle );

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
