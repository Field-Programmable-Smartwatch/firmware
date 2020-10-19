#include <stm32wb55xx.h>
#include <stdint.h>
#include "gpio.h"

void gpio_set_mode(GPIO_TypeDef *gpio_port, uint8_t pin, gpio_mode_t mode)
{    
    gpio_port->MODER &= ~(3 << (pin<<1));
    gpio_port->MODER |= mode << (pin<<1);
}

void gpio_set_output_type(GPIO_TypeDef *gpio_port, uint8_t pin, gpio_output_type_t output_type)
{
    gpio_port->OTYPER &= ~(1 << pin);
    gpio_port->OTYPER |= output_type << pin;
}

void gpio_set_output_speed(GPIO_TypeDef *gpio_port, uint8_t pin, gpio_output_speed_t output_speed)
{
    gpio_port->OSPEEDR &= ~(3 << (pin << 1));
    gpio_port->OSPEEDR |= output_speed << (pin << 1);
}

void gpio_set_pull_resistor(GPIO_TypeDef *gpio_port, uint8_t pin, gpio_pull_resistor_t pull_resistor)
{
    gpio_port->PUPDR &= ~(3 << (pin << 1));
    gpio_port->PUPDR |= pull_resistor << (pin << 1);
}

void gpio_configure_pin(GPIO_TypeDef *gpio_port, uint8_t pin,
                        gpio_mode_t mode,
                        gpio_output_type_t output_type,
                        gpio_output_speed_t output_speed,
                        gpio_pull_resistor_t pull_resistor)
{
    gpio_set_mode(gpio_port, pin, mode);
    gpio_set_output_type(gpio_port, pin, output_type);
    gpio_set_output_speed(gpio_port, pin, output_speed);
    gpio_set_pull_resistor(gpio_port, pin, pull_resistor);
}

uint8_t gpio_read(GPIO_TypeDef *gpio_port, uint8_t pin)
{
    return (gpio_port->IDR >> pin) & 1;
}

void gpio_write(GPIO_TypeDef *gpio_port, uint8_t pin, uint8_t value)
{
    if (value) {
        gpio_port->BSRR |= 1 << pin;
    } else {
        gpio_port->BSRR |= 1 << (pin + 16);
    }
}
