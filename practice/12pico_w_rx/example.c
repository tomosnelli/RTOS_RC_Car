#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

#define PWM_INPUT_PIN 15  // Choose an appropriate GPIO pin

// Function to initialize PWM input
void init_pwm_input(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_config config = pwm_get_default_config();
    
    // Set the PWM clock divider to run at 1MHz
    // System clock is 125MHz, so we divide by 125
    pwm_config_set_clkdiv(&config, 125.0f);
    
    // Set wrap to 20000 for a 20ms period (50Hz)
    pwm_config_set_wrap(&config, 20000);
    
    pwm_init(slice_num, &config, true);
}

// Function to read PWM value
uint16_t read_pwm_value(uint gpio) {
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    uint chan = pwm_gpio_to_channel(gpio);
    return pwm_get_counter(slice_num);
}

int main() {
    stdio_init_all();
    
    init_pwm_input(PWM_INPUT_PIN);
    
    while (1) {
        uint16_t pwm_value = read_pwm_value(PWM_INPUT_PIN);
        
        // Convert PWM value to microseconds (assuming 1MHz clock)
        uint16_t pulse_width_us = pwm_value;
        
        // Check if the pulse width is within the 1000-2000us range
        if (pulse_width_us >= 1000 && pulse_width_us <= 2000) {
            // Map 1000-2000us to 0-100%
            int percentage = (pulse_width_us - 1000) * 100 / 1000;
            printf("PWM Input: %d us, Percentage: %d%%\n", pulse_width_us, percentage);
        } else {
            printf("PWM Input out of range: %d us\n", pulse_width_us);
        }
        
        sleep_ms(100);  // Read every 100ms
    }
    
    return 0;
}
