#include <stm32wb55xx.h>
#include <stdint.h>
#include <stdbool.h>
#include "spi.h"
#include <rcc.h>
#include <log.h>
#include <gpio.h>
#include <string.h>
#include <error.h>

#define SPI_DEVICE_MAX 8

spi_configuration_t g_spi_devices[SPI_DEVICE_MAX];
int32_t g_current_spi_config = -1;

static error_t spi_configure(int32_t spi_handle)
{
    if (spi_handle < 0 ||
        spi_handle >= SPI_DEVICE_MAX ||
        !g_spi_devices[spi_handle].is_open) {
        log_error(ERROR_INVALID, "Invalid SPI handle");
        return ERROR_INVALID;
    }

    spi_configuration_t *spi_dev = &g_spi_devices[spi_handle];
    g_current_spi_config = spi_handle;
    
    uint32_t cr1_value = 0;
    uint32_t cr2_value = 0;
    
    cr1_value |=
        (spi_dev->clock_mode << SPI_CR1_CPHA_Pos) |
        (spi_dev->mode << SPI_CR1_MSTR_Pos) |
        (spi_dev->baud_rate_prescaler << SPI_CR1_BR_Pos) |
        (spi_dev->significant_bit << SPI_CR1_LSBFIRST_Pos);

    if (spi_dev->com_mode == SPI_COM_MODE_FULL_DUPLEX) {
        cr1_value &= ~(1 << SPI_CR1_RXONLY_Pos);
        
    } else if (spi_dev->com_mode == SPI_COM_MODE_HALF_DUPLEX_RECEIVE) {
        cr1_value |= 1 << SPI_CR1_BIDIMODE_Pos;
        cr1_value &= ~(1 << SPI_CR1_BIDIOE_Pos);
        
    } else if (spi_dev->com_mode == SPI_COM_MODE_HALF_DUPLEX_TRANSMIT) {
        cr1_value |= (1 << SPI_CR1_BIDIMODE_Pos) |
            (1 << SPI_CR1_BIDIOE_Pos);
    }

    cr1_value |= SPI_CR1_SSM;
    cr2_value |= SPI_CR2_SSOE;

    cr2_value |= spi_dev->data_size << SPI_CR2_DS_Pos;
    cr2_value |= SPI_CR2_FRXTH;

    SPI1->CR1 = cr1_value;
    SPI1->CR2 = cr2_value;

    SPI1->CR1 |= SPI_CR1_SPE;

    return SUCCESS;
}

static bool spi_tx_buffer_empty()
{
    return (bool)(SPI1->SR & 2);
}

/* static bool spi_rx_buffer_empty() */
/* { */
/*     return (bool)!(SPI1->SR & 1); */
/* } */

static void spi_transmit_8bit(uint8_t data)
{
    while (!spi_tx_buffer_empty()) {}
    *(uint8_t *)&SPI1->DR = data;
}

static void spi_receive_8bit(uint8_t *data)
{
    *data = SPI1->DR;   
}

static int32_t spi_find_free_device()
{
    bool found_free_device = false;
    int32_t spi_handle;

    for (spi_handle = 0; spi_handle < SPI_DEVICE_MAX; spi_handle++) {
        if (g_spi_devices[spi_handle].is_open) {
            continue;
        }

        found_free_device = true;
        break;
    }

    if (found_free_device) {
        return spi_handle;
    }

    return -1;
}

error_t spi_read_write(int32_t spi_handle, void *rdata, uint8_t wdata, uint32_t length)
{
    if (spi_handle < 0 ||
        spi_handle >= SPI_DEVICE_MAX ||
        !g_spi_devices[spi_handle].is_open) {
        log_error(ERROR_INVALID, "Invalid SPI handle");
        return ERROR_INVALID;
    }

    if (g_current_spi_config != spi_handle) {
        spi_configure(spi_handle);
    }
    
    uint8_t *rd = rdata;
    while (length--) {
        spi_transmit_8bit(wdata);
        spi_receive_8bit(rd++);
    }

    return SUCCESS;
}

error_t spi_read(int32_t spi_handle, void *buffer, uint32_t length)
{
    if (spi_handle < 0 ||
        spi_handle >= SPI_DEVICE_MAX ||
        !g_spi_devices[spi_handle].is_open) {
        log_error(ERROR_INVALID, "Invalid SPI handle");
        return ERROR_INVALID;
    }

    if (g_current_spi_config != spi_handle) {
        spi_configure(spi_handle);
    }
    
    uint8_t *d = buffer;
    while (length--) {
        spi_receive_8bit(d++);
    }
    
    return SUCCESS;
}

error_t spi_write(int32_t spi_handle, void *data, uint32_t length)
{
    if (spi_handle < 0 ||
        spi_handle >= SPI_DEVICE_MAX ||
        !g_spi_devices[spi_handle].is_open) {
        log_error(ERROR_INVALID, "Invalid SPI handle");
        return ERROR_INVALID;
    }

    if (g_current_spi_config != spi_handle) {
        spi_configure(spi_handle);
    }
    
    uint8_t *d = data;
    while (length--) {
        spi_transmit_8bit(*d++);
    }

    return SUCCESS;
}

error_t spi_open(spi_configuration_t config, spi_handle_t *spi_handle)
{
    int32_t handle = spi_find_free_device();
    if (handle < 0) {
        log_error(ERROR_NO_MEMORY, "Unable to find free device");
        return ERROR_NO_MEMORY;
    }

    memory_copy(&g_spi_devices[handle], &config, sizeof(spi_configuration_t));
    g_spi_devices[handle].is_open = true;
    spi_configure(handle);

    *spi_handle = handle;
    return SUCCESS;
}

error_t spi_close(int32_t spi_handle)
{
    if (spi_handle < 0 ||
        spi_handle >= SPI_DEVICE_MAX ) {
        log_error(ERROR_INVALID, "Invalid SPI handle");
        return ERROR_INVALID;
    }

    g_spi_devices[spi_handle].is_open = false;

    return SUCCESS;
}

error_t spi_init()
{
    error_t error;
    gpio_configuration_t sck_pin;
    gpio_configuration_t miso_pin;
    gpio_configuration_t mosi_pin;

    rcc_enable_spi1_clock();
    rcc_enable_gpioa_clock();
    
    sck_pin.port = GPIOA;
    sck_pin.pin = 5;
    sck_pin.mode = GPIO_MODE_ALT_FUNC;
    sck_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    sck_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    sck_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    sck_pin.alternative_function = 5;
    error = gpio_configure_pin(sck_pin);
    if (error) {
        log_error(error, "Failed to configure sck pin");
        return error;
    }

    miso_pin.port = GPIOA;
    miso_pin.pin = 6;
    miso_pin.mode = GPIO_MODE_ALT_FUNC;
    miso_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    miso_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    miso_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    miso_pin.alternative_function = 5;
    error = gpio_configure_pin(miso_pin);
    if (error) {
        log_error(error, "Failed to configure miso pin");
        return error;
    }

    mosi_pin.port = GPIOA;
    mosi_pin.pin = 7;
    mosi_pin.mode = GPIO_MODE_ALT_FUNC;
    mosi_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    mosi_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    mosi_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    mosi_pin.alternative_function = 5;
    error = gpio_configure_pin(mosi_pin);
    if (error) {
        log_error(error, "Failed to configure mosi pin");
        return error;
    }

    return SUCCESS;
}

void spi_destroy()
{
    // TODO
    return;
}
