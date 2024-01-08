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
#define ENB_PIN 5
#define IN3_PIN 3
#define IN4_PIN 4

#define PWM_RANGE 100
#define SPEED_LOW 0.0
#define SPEED_HIGH 1.0

pwm_config config;
pwm_config enb_config;
uint pwm_slice_num;
uint enb_slice_num;

int ena_in[] = { IN1_PIN, IN2_PIN };
int enb_in[] = { IN3_PIN, IN4_PIN };

void default_pin_init(){
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
}

void motor_init(){
    // set GPIO pin
    gpio_init(IN1_PIN);
    gpio_init(IN2_PIN);
    gpio_init(IN3_PIN);
    gpio_init(IN4_PIN);

    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_set_dir(IN2_PIN, GPIO_OUT);
    gpio_set_dir(IN3_PIN, GPIO_OUT);
    gpio_set_dir(IN4_PIN, GPIO_OUT);

    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 0);
    gpio_put(IN3_PIN, 0);
    gpio_put(IN4_PIN, 0);

    // configure ENA_PIN
    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    pwm_slice_num = pwm_gpio_to_slice_num(ENA_PIN);

    gpio_set_function(ENB_PIN, GPIO_FUNC_PWM);
    enb_slice_num = pwm_gpio_to_slice_num(ENB_PIN);

    config = pwm_get_default_config();
    pwm_init(pwm_slice_num, &config, true);

    enb_config = pwm_get_default_config();
    pwm_init(enb_slice_num, &enb_config, true);
}

void init_all(){
    sleep_ms(3000);
    stdio_init_all();
    default_pin_init();
    motor_init();  
}

void motor_set_speed(uint en_pin, float speed, pwm_config cfg){
    if(SPEED_LOW <= speed && speed <= SPEED_HIGH)
        pwm_set_gpio_level(en_pin, (cfg.top * speed));
}

void forward(int* pins){
    gpio_put(pins[0], 1);
    gpio_put(pins[1], 0);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

void backward(int* pins){
    gpio_put(pins[0], 0);
    gpio_put(pins[1], 1);
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

void stop(int* pins){
    gpio_put(pins[0], 0);
    gpio_put(pins[1], 0);
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}

//void vSteeringMotorTask(){
//    for(;;){
//        motor_set_speed(0.20);
//        forward();
//        vTaskDelay(pdMS_TO_TICKS(1000));
//        stop();
//        vTaskDelay(pdMS_TO_TICKS(1000));
//        motor_set_speed(0.90);
//        forward();
//        vTaskDelay(pdMS_TO_TICKS(1000));
//        stop();
//        vTaskDelay(pdMS_TO_TICKS(1000));
//    }
//}

void vMainMotorTask(){
    for(int i = 0; i < 10; ++i){
        motor_set_speed(ENB_PIN, 1.0, enb_config);
        forward(enb_in);
        vTaskDelay(pdMS_TO_TICKS(1000));
        motor_set_speed(ENB_PIN, 0.5, enb_config);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void main(){
    init_all();

    sleep_ms(250);

    xTaskCreate(vMainMotorTask, "Main Motor Control Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();
}