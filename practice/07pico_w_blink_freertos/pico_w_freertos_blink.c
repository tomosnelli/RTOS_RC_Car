#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"

void vTaskBlink( void * pvParameters )
{
    if( cyw43_arch_init() )
    {
        printf("Failed to run cyw43_arch_init");
        exit(1);
    }


    for( ;; )
    {
        printf("blink loop");
        cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 );
        vTaskDelay( pdMS_TO_TICKS( 100 ) );
        cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
        vTaskDelay( pdMS_TO_TICKS( 100 ) );
    }

    cyw43_arch_deinit();
}

void vLaunch( void )
{
    BaseType_t xStatus;
    TaskHandle_t blinkHandle;

    xStatus = xTaskCreate( vTaskBlink, "BLINK", 1024, NULL, configMAX_PRIORITIES - 4, &blinkHandle );

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
