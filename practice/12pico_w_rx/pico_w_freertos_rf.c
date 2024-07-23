#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"

/* TASK HANDLES     */
static TaskHandle_t xWaitHandle;
/* END TASK HANDLES */

/**
 * PICO blink control with FreeRTOS event groups.
 * Previously tried queues but kinda got messy
 */

// static void vTaskSendNotification( void * pvParameters )
// {
//     const TickType_t xDelay = pdMS_TO_TICKS( 1000UL ), xMoreDelay = pdMS_TO_TICKS( 5000UL );
// 
//     for ( ;; )
//     {
//         xTaskNotify( xWaitHandle, mainLED_ON, eSetValueWithOverwrite );
//         vTaskDelay( xDelay );
//
//         xTaskNotify( xWaitHandle, mainLED_OFF, eSetValueWithOverwrite );
//         vTaskDelay( xDelay );
// 
//         xTaskNotify( xWaitHandle, mainLED_BLINK, eSetValueWithOverwrite );
//         vTaskDelay( xMoreDelay );
//     }
// }

static void vTaskReceiveNotification( void * pvParameters )
{
    uint32_t ulNotifiedValue;

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
                int state = 1;

                for( int i = 0; i < 6; i++)
                {
                    cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, state );
                    vTaskDelay( pdMS_TO_TICKS( 100 ) );
                    state ^= 1;
                }

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

void vTaskRXTask( void * pvParameters )
{
    if( cyw43_arch_init() )
    {
        // failed
        for( ;; );
    }

    // READ
    for( ;; )
    {
        assert(pwm_gpio_to_channel(gpio) == PWM_CHAN_B);
    uint slice_num = pwm_gpio_to_sliece_num(gpio);
}

int main()
{
    stdio_init_all();

    BaseType_t xStatus = pdPASS;

    xStatus &= xTaskCreate( vTaskReceiveNotification, "WaitNotify", 1024, NULL, configMAX_PRIORITIES - 5, &xWaitHandle );
    xStatus &= xTaskCreate( vTaskBLEServer, "CLASSIC_SETUP", 1024, NULL, configMAX_PRIORITIES - 3, NULL );

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

