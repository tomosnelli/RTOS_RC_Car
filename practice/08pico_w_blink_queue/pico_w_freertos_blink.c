#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define LED_QUEUE_MAX 5

typedef struct LED_Command
{
    LED_Operation_t * command;
    size_t uSize;
} LED_Command_t;

void vTaskSendLED_Command( void * pvParameters )
{
    // only send once
    BaseType_t xStatus;
    LED_Command_t myCommand = { pairing, sizeof( pairing )/sizeof( LED_Operation_t ) };

    for( ;; )
    {
        xStatus = xQueueSend( xLED_Queue, ( void * ) &myCommand, pdMS_TO_TICKS( 250 ) );

        if( xStatus != pdTRUE )
        {
            // failed....
        }

        vTaskDelay( pdMS_TO_TICKS( 3000 ) );
    }
}

// set this to low prio
void vTaskBLE_Pairing( void * pvParameters )
{
    for( ;; )
    {
        xTaskNotifyWait( 0, 0, &ulNotification, portMAX_DEPLAY );

        if( ulNotification == START_TASK_NOTIFICATION )
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
        }
    }
}

/* Gate keeper task that will receive commands to the Queue.
   Data should be 1 cycle of blinks.
   For example, if you just want to blink it on and off:
   1
*/

void vTaskCYW_LED( void * pvParameters )
{
    for( ;; )
    {
        xTaskNotifyWait( 0, 0x00, &ulNotifiedValue, portMAX_DEPLAY );

        if( ( ulNotifiedValue & 0x01 &0x01) != 0 )
        {
            
        }
    }
}

void vLaunch( void )
{
    BaseType_t xStatus;
    TaskHandle_t blinkHandle;

    xStatus = xTaskCreate( vTaskSendLED_Command, "SEND", 1024, NULL, configMAX_PRIORITIES - 5, NULL );
    xStatus = xTaskCreate( vTaskCYW_LED, "RECEIVE", 1024, NULL, configMAX_PRIORITIES - 3, NULL );

    if( xStatus != pdPASS )
    {
        printf("Error: xTaskCreate failed");
        exit(1);
    }
    else
    {
        vTaskCoreAffinitySet(blinkHandle, 1);
        vTaskStartScheduler();
    }
}

int main()
{
    stdio_init_all();
    printf("init stuff");

    vLaunch();

    for( ;; );

    return 0;
}
