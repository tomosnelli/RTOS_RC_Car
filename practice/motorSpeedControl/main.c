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

void motor_init(){
    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);

    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);

    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 0);

    // configure ENA_PIN
    /*
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 25.0f); // Set 
    pwm_config_set_wrap(&motor_ena_config, PWM_RANGE);
    pwm_init(ENA_PIN, &motor_ena_config, true);
    pwm_set_enabled(ENA_PIN, true);
    pwm_set_gpio_level(ENA_PIN, true);
    */

    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(ENA_PIN);
    pwm_set_wrap(slice_num, PWM_RANGE);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
    pwm_set_enabled(slice_num, true);
}

void motor_set_speed(int speed){
    uint slice_num = pwm_gpio_to_slice_num(ENA_PIN);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, speed); // set PWM duty cycle (0-100%)
}

void forward(){
    gpio_put(IN1_PIN, 1);
    gpio_put(IN2_PIN, 0);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

void backward(){
    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 1);
}

void stop(){
    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 0);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

void vMotorTask(){
    for(;;){
        forward();
        vTaskDelay(pdMS_TO_TICKS(1000));
        stop();
        vTaskDelay(pdMS_TO_TICKS(1000));
        motor_set_speed(50);
        forward();
        vTaskDelay(pdMS_TO_TICKS(1000));
        stop();
        motor_set_speed(99);
    }
}

void main(){
    sleep_ms(3000);
    stdio_init_all();
    motor_init();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    sleep_ms(250);

    xTaskCreate(vMotorTask, "Motor Control Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();
}