#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "FreeRTOS.h"
#include "task.h"
#include <limits.h>

/* TASK HANDLES     */
static TaskHandle_t xWaitHandle;
/* END TASK HANDLES */

/* NOTIFY */
#define mainLED_ON    0x00
#define mainLED_OFF   0x01
#define mainLED_BLINK 0x02

/* PWM PINS */
#define PWM_CHANNEL_1 17
#define PWM_CHANNEL_2 19

/* PWM READ */
// #define SIGNAL_MID 32767

// sweet spot?
// #define SIGNAL_MID 19000
#define SIGNAL_MID 0.0f

// #define SIGNAL_MID 0x7fff
// #define SIGNAL_MID 0x7fff
// #define SIGNAL_MID 0xffff
// #define SIGNAL_MID 0x00

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
            // vTaskDelay( pdMS_TO_TICKS( 1000UL ) );
        }
        else if( ( ulNotifiedValue & mainLED_OFF ) != 0 )
        {
            cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
            // vTaskDelay( pdMS_TO_TICKS( 1000UL ) );
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

                // vTaskDelay( pdMS_TO_TICKS( 500 ) );

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

float measure_duty_cycle(uint gpio) {
    // Only the PWM B pins can be used as inputs.
    assert(pwm_gpio_to_channel(gpio) == PWM_CHAN_B);
    uint slice_num = pwm_gpio_to_slice_num(gpio);

    // Count once for every 100 cycles the PWM B input is high
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&cfg, PWM_DIV_B_HIGH);
    pwm_config_set_clkdiv(&cfg, 100);
    pwm_init(slice_num, &cfg, false);
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    pwm_set_enabled(slice_num, true);
    sleep_ms(10);
    pwm_set_enabled(slice_num, false);
    float counting_rate = clock_get_hz(clk_sys) / 100;
    float max_possible_count = counting_rate * 0.01;
    return pwm_get_counter(slice_num) / max_possible_count;
}

void vTaskRX( void * pvParameters )
{
    uint16_t state;
    if( cyw43_arch_init() )
    {
        // failed
        for( ;; );
    }

    // get reading from each pin
    for( ;; ){

        // i <= PMW.. but set to < for dev
        for( int i = PWM_CHANNEL_2; i < PWM_CHANNEL_2 + 1; i += 2 )
        {
            // ready pin
            uint16_t measured_duty_cycle1 = measure_duty_cycle(i);
            // vTaskDelay( pdMS_TO_TICKS( 10 ) );
            // uint16_t measured_duty_cycle2 = measure_duty_cycle(i);
            // take action based on reading

            // float pin_value = measured_duty_cycle1 > measured_duty_cycle2 ? measured_duty_cycle1 : measured_duty_cycle2;
            float pin_value = measured_duty_cycle1;

            if( (pin_value * 100.f) == SIGNAL_MID )
            {
                // input turned off, stop motor or servo
                // xTaskNotify( xWaitHandle, mainLED_OFF, eSetValueWithOverwrite );
                cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
            }
            else if( (pin_value * 100.f) > SIGNAL_MID )
            {
                // Move forward
                xTaskNotify( xWaitHandle, mainLED_BLINK, eSetValueWithOverwrite );
            }
            else if ( (pin_value * 100.f) < SIGNAL_MID )
            {
                // Move backwards
                // xTaskNotify( xWaitHandle, mainLED_ON, eSetValueWithOverwrite );
                cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 );
            }
        }
    }
}

int main()
{
    stdio_init_all();

    BaseType_t xStatus = pdPASS;

    xStatus &= xTaskCreate( vTaskReceiveNotification, "WaitNotify", 1024, NULL, configMAX_PRIORITIES - 5, &xWaitHandle );
    xStatus &= xTaskCreate( vTaskRX, "CLASSIC_SETUP", 1024, NULL, configMAX_PRIORITIES - 3, NULL );

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

