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

void configure()
{
    gpio_configuration_t gpiob_pin7_config;

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    gpiob_pin7_config.port = GPIOB;
    gpiob_pin7_config.pin = 7;
    gpiob_pin7_config.mode = GPIO_MODE_OUTPUT;
    gpiob_pin7_config.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpiob_pin7_config.output_speed = GPIO_OUTPUT_SPEED_LOW;
    gpiob_pin7_config.pull_resistor = GPIO_PULL_RESISTOR_NONE;

    gpio_configure_pin(gpiob_pin7_config);
}

void main()
{
    configure();
    
    while (1) {
        led_on();
        for (uint32_t i = 0; i < 400000; i++);
        led_off();
        for (uint32_t i = 0; i < 400000; i++);
    }
}
