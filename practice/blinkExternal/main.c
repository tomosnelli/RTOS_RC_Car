#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#define LED_PIN 0

void vBlinkTask(){
    for(;;){
        gpio_put(LED_PIN, 1);
        vTaskDelay(250);
        gpio_put(LED_PIN, 0);
        vTaskDelay(250);
    }
}

void main(){
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    /*
        Documentation on xTaskCreate
        https://www.freertos.org/a00125.html

        
        BaseType_t xTaskCreate(    TaskFunction_t pvTaskCode,
                                   const char * const pcName,
                                   configSTACK_DEPTH_TYPE usStackDepth,
                                   void *pvParameters,
                                   UBaseType_t uxPriority,
                                   TaskHandle_t *pxCreatedTask
                                 );
    */
    xTaskCreate(vBlinkTask, "Blink Task", 128, NULL, 1, NULL);
    vTaskStartScheduler();
}