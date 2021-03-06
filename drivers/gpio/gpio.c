#include <stm32wb55xx.h>
#include <stdint.h>
#include "gpio.h"
#include <error.h>

error_t gpio_set_mode(GPIO_TypeDef *gpio_port, uint8_t pin, gpio_mode_t mode)
{
    if (pin > 15 || !gpio_port) {
        return ERROR_INVALID;
    }

    gpio_port->MODER &= ~(3 << (pin<<1));
    gpio_port->MODER |= mode << (pin<<1);

    return SUCCESS;
}

error_t gpio_set_output_type(GPIO_TypeDef *gpio_port, uint8_t pin, gpio_output_type_t output_type)
{
    if (pin > 15 || !gpio_port) {
        return ERROR_INVALID;
    }

    gpio_port->OTYPER &= ~(1 << pin);
    gpio_port->OTYPER |= output_type << pin;

    return SUCCESS;
}

error_t gpio_set_output_speed(GPIO_TypeDef *gpio_port, uint8_t pin, gpio_output_speed_t output_speed)
{
    if (pin > 15 || !gpio_port) {
        return ERROR_INVALID;
    }

    gpio_port->OSPEEDR &= ~(3 << (pin << 1));
    gpio_port->OSPEEDR |= output_speed << (pin << 1);

    return SUCCESS;
}

error_t gpio_set_pull_resistor(GPIO_TypeDef *gpio_port, uint8_t pin, gpio_pull_resistor_t pull_resistor)
{
    if (pin > 15 || !gpio_port) {
        return ERROR_INVALID;
    }

    gpio_port->PUPDR &= ~(3 << (pin << 1));
    gpio_port->PUPDR |= pull_resistor << (pin << 1);

    return SUCCESS;
}

error_t gpio_set_alternative_function(GPIO_TypeDef *gpio_port, uint8_t pin, uint8_t alternative_function)
{
    if (pin > 15 || !gpio_port) {
        return ERROR_INVALID;
    }
    
    if (pin < 8) {
        gpio_port->AFR[0] |= alternative_function << (pin << 2);
    } else {
        gpio_port->AFR[1] |= alternative_function << (pin << 2);
    }

    return SUCCESS;
}

error_t gpio_read(GPIO_TypeDef *gpio_port, uint8_t pin, uint8_t *value)
{
    if (!gpio_port || pin > 15) {
        return ERROR_INVALID;
    }
    *value = (gpio_port->IDR >> pin) & 1;
    return SUCCESS;
}

error_t gpio_write(GPIO_TypeDef *gpio_port, uint8_t pin, uint8_t value)
{
    if (!gpio_port || pin > 15) {
        return ERROR_INVALID;
    }

    if (value) {
        gpio_port->BSRR |= 1 << pin;
    } else {
        gpio_port->BSRR |= 1 << (pin + 16);
    }

    return SUCCESS;
}

error_t gpio_configure_pin(gpio_configuration_t config)
{
    error_t error;
    error = gpio_set_mode(config.port, config.pin, config.mode);
    if (error) {
        return error;
    }
    
    error = gpio_set_output_type(config.port, config.pin, config.output_type);
    if (error) {
        return error;
    }
    
    error = gpio_set_output_speed(config.port, config.pin, config.output_speed);
    if (error) {
        return error;
    }
    
    error = gpio_set_pull_resistor(config.port, config.pin, config.pull_resistor);
    if (error) {
        return error;
    }
    
    error = gpio_set_alternative_function(config.port, config.pin, config.alternative_function);
    return error;
}
