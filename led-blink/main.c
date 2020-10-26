#include <stm32wb55xx.h>
#include <stdint.h>

#include <gpio.h>

void led_on()
{
    gpio_write(GPIOB, 7, 1);
}

void led_off()
{
    gpio_write(GPIOB, 7, 0);
}

void main()
{
    // Enable GPIO PORT A Clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    gpio_configure_pin(GPIOB, 7,
                       GPIO_MODE_OUTPUT,
                       GPIO_OUTPUT_TYPE_PUSH_PULL,
                       GPIO_OUTPUT_SPEED_LOW,
                       GPIO_PULL_RESISTOR_NONE);
    while (1) {
        led_on();
        for (uint32_t i = 0; i < 400000; i++);
        led_off();
        for (uint32_t i = 0; i < 400000; i++);
    }
}
