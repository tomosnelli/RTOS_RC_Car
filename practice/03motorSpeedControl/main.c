/*
    Practice Motor Control with L298N and pico
*/
#include "pico/stdlib.h"
//#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"

#define ENA_PIN 2
#define IN1_PIN 1
#define IN2_PIN 0

#define PWM_RANGE 100
#define SPEED_LOW 0.0
#define SPEED_HIGH 1.0

pwm_config config;
uint pwm_slice_num;

void default_pin_init(){
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void motor_init(){
    // set GPIO pin
    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);

    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);

    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 0);

    // configure ENA_PIN
    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    pwm_slice_num = pwm_gpio_to_slice_num(ENA_PIN);

    config = pwm_get_default_config();
    pwm_init(pwm_slice_num, &config, true);
}

void init_all(){
    sleep_ms(3000);
    stdio_init_all();
    default_pin_init();
    motor_init();  
}

void motor_set_speed(float speed){
    if(SPEED_LOW <= speed && speed <= SPEED_HIGH)
        pwm_set_gpio_level(ENA_PIN, (config.top * speed));
}

void forward(){
    gpio_put(IN1_PIN, 1);
    gpio_put(IN2_PIN, 0);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

void backward(){
    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 1);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

void stop(){
    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 0);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

void vMotorTask(){
    for(;;){
        motor_set_speed(0.20);
        forward();
        vTaskDelay(pdMS_TO_TICKS(1000));
        stop();
        vTaskDelay(pdMS_TO_TICKS(1000));
        motor_set_speed(0.90);
        forward();
        vTaskDelay(pdMS_TO_TICKS(1000));
        stop();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void main(){
    init_all();

    sleep_ms(250);

    xTaskCreate(vMotorTask, "Motor Control Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();
}