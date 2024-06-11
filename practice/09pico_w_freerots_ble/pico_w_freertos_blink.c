#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#define mainLED_ON_BIT      ( 1UL << 0UL )
#define mainLED_OFF_BIT     ( 1UL << 1UL )
#define mainLED_BLINK_BIT   ( 1UL << 2UL )

static EventGroupHandle_t xEventGroup;
static TaskHandle_t SettingHandle;
static TaskHandle_t ReadingHandle;

/**
 * PICO blink control with FreeRTOS event groups.
 * Previously tried queues but kinda got messy
 */

static void vEventBitSettingTask( void * pvParameters )
{
    const TickType_t xDelay = pdMS_TO_TICKS( 1000UL ), xDontBlock = 0;

    for( ;; )
    {
        vTaskDelay( xDelay );

        // set bit to turn on 
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

                // immediately exit
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
        // vTaskCoreAffinitySet(SettingHandle, 1);
        // vTaskCoreAffinitySet(ReadingHandle, 0);
        vTaskStartScheduler();
    }

    for( ;; );

    return 0;
}
