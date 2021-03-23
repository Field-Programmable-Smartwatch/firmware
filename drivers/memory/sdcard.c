#include <stm32wb55xx.h>
#include <debug.h>
#include <sdcard.h>
#include <spi.h>
#include <gpio.h>
#include <crc7.h>
#include <rcc.h>
#include <string.h>
#include <systick_timer.h>
#include <error.h>

static spi_handle_t g_spi_handle;

static inline error_t sdcard_select()
{
    return gpio_write(GPIOB, 6, 0);
}

static inline error_t sdcard_deselect()
{
    return gpio_write(GPIOB, 6, 1);
}

static error_t sdcard_get_response(void *response, uint32_t length)
{
    return spi_read_write(g_spi_handle, response, 0xff, length);
}

static error_t sdcard_wait_till_ready()
{
    uint8_t timeout = 0;
    uint8_t resp = 0;
    while (resp != 0xff && timeout < 50) {
        error_t error = sdcard_get_response(&resp, 1);
        if (error) {
            return error;
        }
        timeout++;
    }

    if (timeout == 50) {
        return ERROR_TIMEOUT;
    }

    return SUCCESS;
}

static error_t sdcard_send_command(uint8_t cmd, uint32_t arg, uint8_t *response)
{
    error_t error;
    uint8_t packet[6];
    uint8_t msg[] = {
        cmd | 0x40,
        (uint8_t)(arg >> 24),
        (uint8_t)(arg >> 16),
        (uint8_t)(arg >> 8),
        (uint8_t)(arg)
    };
    uint8_t crc = crc7_calculate(msg, 5) + 1;

    memcpy(packet, msg, 5);
    memcpy(&packet[5], &crc, 1);
    
    error = sdcard_wait_till_ready();
    if (error) {
        return error;
    }

    error = spi_write(g_spi_handle, packet, 6);
    if (error) {
        return error;
    }

    uint8_t count = 10;
    do {
        error_t error = sdcard_get_response(response, 1);
        if (error) {
            return error;
        }
        count--;
    } while ((*response & 0x80) && count);

    if (!count) {
        return ERROR_TIMEOUT;
    }
    return SUCCESS;
}

error_t sdcard_read_block(uint32_t addr, void *buffer)
{
    error_t error;
    uint8_t response;
    uint8_t *data = buffer;
    uint16_t crc;

    sdcard_select();

    do {
        error_t error = sdcard_send_command(17, addr, &response);
        if (error) {
            goto exit;
        }
    } while (response);
    
    // Wait for data block
    uint8_t temp = 0xff;
    while (temp == 0xff) {
        error = spi_read_write(g_spi_handle, &temp, 0xff, 1);
        if (error) {
            goto exit;
        }
    }

    // Get data block
    error = sdcard_get_response(data, 512);
    if (error) {
        goto exit;
    }

    error = sdcard_get_response(&crc, 2);
    
 exit:
    sdcard_deselect();
    return error;
}

error_t sdcard_write_block(uint32_t addr, void *buffer)
{
    error_t error;
    uint8_t response;
    
    sdcard_select();
    do {
        error = sdcard_send_command(24, addr, &response);
        if (error) {
            goto exit;
        }
    } while (response);
    
    uint8_t msg[515];
    uint8_t data_start_token = 0xfe;
    uint16_t crc = 0xffff;
    uint8_t resp;
    
    memcpy(&msg[0], &data_start_token, 1);
    memcpy(&msg[1], buffer, 512);
    memcpy(&msg[513], &crc, 2);

    error = spi_write(g_spi_handle, msg, 515);
    if (error) {
        goto exit;
    }

    while ((resp & 0x1f) != 0x5) {
        debug_print("Failed to get block write response %u\r\n", resp);
        error = sdcard_get_response(&resp, 1);
        if (error) {
            goto exit;
        }
    }

    error = sdcard_send_command(13, 0, &response);
    if (error) {
        debug_print("failed to get r2 resp\r\n");
        goto exit;
    }
    error = sdcard_get_response(&resp, 1);

 exit:
    sdcard_deselect();
    return error;
}

error_t sdcard_init()
{
    error_t error;
    gpio_configuration_t cs_pin;
    spi_configuration_t spi1;

    // Enable clock for GPIO port B
    rcc_enable_gpiob_clock();
    
    // Configure Chip Select pin
    cs_pin.port = GPIOB;
    cs_pin.pin = 6;
    cs_pin.mode = GPIO_MODE_OUTPUT;
    cs_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    cs_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    cs_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    error = gpio_configure_pin(cs_pin);
    if (error) {
        return error;
    }

    error = gpio_write(cs_pin.port, cs_pin.pin, 1);
    if (error) {
        return error;
    }

    // Configure SPI interface
    spi1.clock_mode = SPI_CLOCK_MODE_0;
    spi1.mode = SPI_MODE_MASTER;
    spi1.baud_rate_prescaler = SPI_BAUD_RATE_PRESCALER_64;
    spi1.significant_bit = SPI_SIGNIFICANT_BIT_MSB;
    spi1.com_mode =  SPI_COM_MODE_FULL_DUPLEX;
    spi1.data_size = SPI_DATA_SIZE_8BIT;
    error = spi_open(spi1, &g_spi_handle);
    if (error) {
        return error;
    }

    for (uint32_t i = 0; i < 100; i++) {
        uint8_t msg = 0xff;
        error = spi_write(g_spi_handle, &msg, 1);
        if (error) {
            return error;
        }
    }

    sdcard_select();

    uint8_t timeout = 100;
    uint8_t response;
    do {
        error = sdcard_send_command(0, 0, &response);
        if (error) {
            goto exit;
        }
        timeout--;
    } while (response != 0x01 && timeout);

    // Check if command timed out
    if (timeout == 0) {
        error = ERROR_TIMEOUT;
        goto exit;
    }

    error = sdcard_send_command(8, 0x1AA, &response);
    if (error) {
        goto exit;
    }
    if (response & 0x4) {
        error = ERROR_IO;
        goto exit;
    }

    timeout = 100;
    do {
        error = sdcard_send_command(55, 0, &response); // TODO: Check response value here?
        if (error) {
            goto exit;
        }
        
        error = sdcard_send_command(41, 0x40000000, &response);
        if (error) {
            goto exit;
        }

        timeout--;
    } while (response != 0 && timeout);

    if (timeout == 0) {
        error = ERROR_TIMEOUT;
    }
 exit:
    sdcard_deselect();
    return error;
}
