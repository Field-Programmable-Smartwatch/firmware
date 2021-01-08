#include <stm32wb55xx.h>
#include <stdint.h>
#include <stdbool.h>
#include "spi.h"
#include <debug.h>
#include <gpio.h>

void spi_configure(SPI_TypeDef *spi, spi_configuration_t config)
{
    uint32_t cr1_value = 0;
    uint32_t cr2_value = 0;

    gpio_configuration_t sck_pin;
    gpio_configuration_t miso_pin;
    gpio_configuration_t mosi_pin;

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    
    sck_pin.port = GPIOA;
    sck_pin.pin = 5;
    sck_pin.mode = GPIO_MODE_ALT_FUNC;
    sck_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    sck_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    sck_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    sck_pin.alternative_function = 5;
    gpio_configure_pin(sck_pin);

    miso_pin.port = GPIOA;
    miso_pin.pin = 6;
    miso_pin.mode = GPIO_MODE_ALT_FUNC;
    miso_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    miso_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    miso_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    miso_pin.alternative_function = 5;
    gpio_configure_pin(miso_pin);

    mosi_pin.port = GPIOA;
    mosi_pin.pin = 7;
    mosi_pin.mode = GPIO_MODE_ALT_FUNC;
    mosi_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    mosi_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    mosi_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    mosi_pin.alternative_function = 5;
    gpio_configure_pin(mosi_pin);
    
    cr1_value |=
        (config.clock_mode << SPI_CR1_CPHA_Pos) |
        (config.mode << SPI_CR1_MSTR_Pos) |
        (config.baud_rate_prescaler << SPI_CR1_BR_Pos) |
        (config.significant_bit << SPI_CR1_LSBFIRST_Pos);

    if (config.com_mode == SPI_COM_MODE_FULL_DUPLEX) {
        cr1_value &= ~(1 << SPI_CR1_RXONLY_Pos);
        
    } else if (config.com_mode == SPI_COM_MODE_HALF_DUPLEX_RECEIVE) {
        cr1_value |= 1 << SPI_CR1_BIDIMODE_Pos;
        cr1_value &= ~(1 << SPI_CR1_BIDIOE_Pos);
        
    } else if (config.com_mode == SPI_COM_MODE_HALF_DUPLEX_TRANSMIT) {
        cr1_value |= (1 << SPI_CR1_BIDIMODE_Pos) |
            (1 << SPI_CR1_BIDIOE_Pos);
    }

    cr1_value |= SPI_CR1_SSM;
    cr2_value |= SPI_CR2_SSOE;

    cr2_value |= config.data_size << SPI_CR2_DS_Pos;
    cr2_value |= SPI_CR2_FRXTH;

    spi->CR1 = cr1_value;
    spi->CR2 = cr2_value;

    spi->CR1 |= SPI_CR1_SPE;
}

static bool spi_tx_buffer_empty(SPI_TypeDef *spi)
{
    return (bool)(spi->SR & 2);
}

void spi_set_nss(SPI_TypeDef *spi, uint8_t value)
{
    spi->CR1 |= value << SPI_CR1_SSI_Pos;
}

void spi_transmit(SPI_TypeDef *spi, uint8_t data)
{
    while (!spi_tx_buffer_empty(spi)) {}
    *(uint8_t *)&spi->DR = data;
}

void spi_transmit_buffer(SPI_TypeDef *spi, uint8_t *msg, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++) {
        spi_transmit(spi, msg[i]); 
    }
}
