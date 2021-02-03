#include <stm32wb55xx.h>
#include <debug.h>
#include <sdcard.h>
#include <spi.h>
#include <gpio.h>
#include <crc7.h>
#include <rcc.h>
#include <string.h>
#include <systick_timer.h>

static int32_t g_spi_handle = -1;

static inline void sdcard_select()
{
    gpio_write(GPIOB, 6, 0);
}

static inline void sdcard_deselect()
{
    gpio_write(GPIOB, 6, 1);
}

static void sdcard_get_response(void *response, uint32_t length)
{
    spi_read_write(g_spi_handle, response, 0xff, length);
}

static void sdcard_wait_till_ready()
{
    uint8_t resp = 0;
    while (resp != 0xff) {
        sdcard_get_response(&resp, 1);
    }
}

static uint8_t sdcard_send_command(uint8_t cmd, uint32_t arg)
{
    sdcard_wait_till_ready();
    uint8_t msg[] = {
        cmd | 0x40,
        (uint8_t)(arg >> 24),
        (uint8_t)(arg >> 16),
        (uint8_t)(arg >> 8),
        (uint8_t)(arg)
    };

    uint8_t crc = crc7_calculate(msg, 5) + 1;
    spi_write(g_spi_handle, msg, 5);
    spi_write(g_spi_handle, &crc, 1);

    uint8_t ret = 0xff;
    uint8_t count = 10;
    while ((ret & 0x80) && count) {
        sdcard_get_response(&ret, 1);
        count--;
    }
    return ret;
}

void sdcard_read_block(uint32_t addr, void *buffer)
{
    sdcard_select();
    uint8_t *data = buffer;
    uint16_t crc;
    while (sdcard_send_command(17, addr)) {
        debug_print("Failed to read block\r\n");
    }
    
    // Wait for data block
    uint8_t temp = 0xff;
    while (temp == 0xff) {
        spi_read_write(g_spi_handle, &temp, 0xff, 1);
    }

    // Get data block
    sdcard_get_response(data, 512);
    sdcard_get_response(&crc, 2);
    sdcard_deselect();
}

void sdcard_write_block(uint32_t addr, void *buffer)
{
    sdcard_select();
    while (sdcard_send_command(24, addr)) {
        debug_print("Failed to write block\r\n");
    }
    
    uint8_t msg[515];
    uint8_t data_start_token = 0xfe;
    uint16_t crc = 0xffff;
    uint8_t resp;
    
    memcpy(&msg[0], &data_start_token, 1);
    memcpy(&msg[1], buffer, 512);
    memcpy(&msg[513], &crc, 2);

    spi_write(g_spi_handle, msg, 515);

    while ((resp & 0x1f) != 0x5) {
        debug_print("Failed to get block write response %u\r\n", resp);
        sdcard_get_response(&resp, 1);
    }

    if (sdcard_send_command(13, 0)) {
        debug_print("failed to get r2 resp\r\n");
        sdcard_deselect();
        return;
    }
    sdcard_get_response(&resp, 1);
    sdcard_deselect();
}

void sdcard_init()
{
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
    gpio_configure_pin(cs_pin);
    gpio_write(cs_pin.port, cs_pin.pin, 1);

    // Configure SPI interface
    spi1.clock_mode = SPI_CLOCK_MODE_0;
    spi1.mode = SPI_MODE_MASTER;
    spi1.baud_rate_prescaler = SPI_BAUD_RATE_PRESCALER_64;
    spi1.significant_bit = SPI_SIGNIFICANT_BIT_MSB;
    spi1.com_mode =  SPI_COM_MODE_FULL_DUPLEX;
    spi1.data_size = SPI_DATA_SIZE_8BIT;
    g_spi_handle = spi_open(spi1);

    for (uint32_t i = 0; i < 100; i++) {
        uint8_t msg = 0xff;
        spi_write(g_spi_handle, &msg, 1);
    }

    sdcard_select();
    uint8_t test = 0;
    while ((test = sdcard_send_command(0, 0)) != 0x01) {
        debug_print("sdcard_init: retrying GO_IDLE_STATE cmd %u\r\n", test);
    }
    
    if (sdcard_send_command(8, 0x1AA) & 0x4) {
        debug_print("sdcard_init: retrying SEND_IF_COND cmd\r\n");
        return;
    }
    
    uint8_t status = 1;
    while (status != 0) {
        debug_print("sdcard_init: retrying SD_SEND_OP_COND cmd\r\n");
        sdcard_send_command(55, 0);
        status = sdcard_send_command(41, 0x40000000);
    }

    sdcard_deselect();
}
