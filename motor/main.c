/*
    Practice Motor Control with L298N and pico
*/
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#define ENA_PIN 6
#define IN1_PIN 4
#define IN2_PIN 5

void motor_init(){
    gpio_init(ENA_PIN);
    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);

    gpio_set_dir(ENA_PIN, GPIO_OUT);
    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);

    gpio_put(ENA_PIN, 1);
    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 0);
}

void forward(){
    gpio_put(IN1_PIN, 1);
    gpio_put(IN2_PIN, 0);
}

void backward(){
    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 1);
}

void stop(){
    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 0);
}

void vMotorTask(){
    for(;;){
        vTaskDelay(1000);
        forward();
        vTaskDelay(250);
        backward();
    }
}


void main(){
    motor_init();

    xTaskCreate(vMotorTask, "Motor Task", 128, NULL, 1, NULL);
    vTaskStartScheduler();
}