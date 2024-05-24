#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"



int main()
{
    stdio_init_all();
    cyw43_arch_init(); 

    for( ;; )
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(250);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(250);
    }

    return 0;
}
