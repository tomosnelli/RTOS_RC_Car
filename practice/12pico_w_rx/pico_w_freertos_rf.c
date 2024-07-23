#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
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

float measure_duty_cycle(uint pin)
{
    // Only PWM B pins can be used as inputs
    assert(pwm_gpio_to_channel(pin) == PWM_CHAN_B);
    uint sliece_num = pwm_gpio_to_slice_num(pin);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_HIGH);
    pwm_config_set_clkdiv(&cfg, 100);
    pwm_init(sliece_num, &cfg, false);
    gpio_set_function(pin, GPIO_FUNC_PWM);

    pwm_set_enabled(slice_num, true);
    sleep_ms(10);
    pwm_set_enabled(sliece_num, false);
    float counting_rate = clock_get_hz(clk_sys) / 100;
    float max_possible_count = counting_rate * 0.01;
    return pwm_get_counter(slice_num) / max_possible_count;
}

void vTaskRX( void * pvParameters )
{
    if( cyw43_arch_init() )
    {
        // failed
        for( ;; );
    }

    // get reading from each pin
    for( ;; ){
        for( int i = 0; i < 2; i++ )
        {
            // ready pin
            float measured_duty_cycle1 = measure_duty_cycle(i);
            float measured_duty_cycle2 = measure_duty_cycle(i);
            // take action based on reading

            float pin_value = measured_duty_cycle1 > measured_duty_cycle2 ? measured_duty_cycle1 : measured_duty_cycle2;
            pin_value *= 100;

            if( pin_value < 0.1 )
            {
                // input turned off, stop motor or servo
            }
            else if( pin_value < 10 || pin_value > 20 )
            {
                // Input is invalid
            }
            else
            {
                // change motor status
            }
        }
    }
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

