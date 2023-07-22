/*
    practice radio signal handling
*/
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "FreeRTOS.h"
#include "task.h"

#define LED_PIN 25
#define RADIO_CE_PIN 17
#define RADIO_CSN_PIN 16

void radio_init() {
    // Implement radio module initialization here
    // Set up SPI and any required settings for NRF24L01
}

void led_task(void* pvParameters) {
    (void) pvParameters;
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    while (1) {
        gpio_put(LED_PIN, 1); // Turn on LED
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 500 milliseconds
        gpio_put(LED_PIN, 0); // Turn off LED
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay 500 milliseconds
    }
}

void radio_task(void* pvParameters) {
    (void) pvParameters;

    while (1) {
        // Implement radio signal reception here
        // Read data from the radio module

        // Process the received data and take appropriate actions
        // For example, if the received data is a specific command, toggle the LED
    }
}

int main() {
    stdio_init_all();

    // Initialize the radio module
    radio_init();

    // Create tasks for LED blinking and radio signal reception
    xTaskCreate(led_task, "LED Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(radio_task, "Radio Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    return 0;
}
