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

void motor_init(){
    //init gpio
    gpio_init(ENA_PIN);
    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);

    //set gpio pin directions
    gpio_set_dir(ENA_PIN, GPIO_OUT);
    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);

    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 0);

    // enable the motor by setting the ENA pin hight
    gpio_put(ENA_PIN, 1);

/*
    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(ENA_PIN);
    pwm_init(slice_num);
    pwm_set_wrap(slice_num, 100); // PWM period 100 cycles
    pwm_set_clkdiv(slice_num, 1.0f); // PWM clock divider 1.0
*/
}

/*
void motor_set_speed(float speed){
    uint slice_num = pwm_gpio_to_slice_num(ENA_PIN);
    pwm_set_chan_level(slice_num, ENA_PIN, speed * 100); // set PWM duty cycle (0-100%)
}
*/

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
        forward();
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        vTaskDelay(2000);
        stop();
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        vTaskDelay(1000);
    }
}


void main(){
    stdio_init_all();
    motor_init();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    sleep_ms(250);

    xTaskCreate(vMotorTask, "Motor Task", 128, NULL, 1, NULL);
    vTaskStartScheduler();
}