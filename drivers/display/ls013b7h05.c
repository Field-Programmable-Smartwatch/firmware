#include <stm32wb55xx.h>
#include <stdint.h>
#include <string.h>

#include <gpio.h>
#include <debug.h>
#include <spi.h>
#include <display.h>
#include <rcc.h>

#define DISPLAY_WIDTH 144
#define DISPLAY_HEIGHT 168

uint8_t vcom = 0;
uint32_t display_buffer_size = (DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8;
uint8_t display_buffer[(DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8];

display_draw_attr_t g_draw_attr = DISPLAY_DRAW_ATTR_NORMAL;
int32_t g_spi_handle = -1;

static uint8_t reverse_byte(uint8_t value)
{
    uint32_t v = value;
    __asm__("rbit %0, %0"
            : "=r" (v)
            :);
    return v >> 24;
}

void display_init()
{
    gpio_configuration_t cs_pin;
    spi_configuration_t spi1;

    // Enable clock for GPIO port B
    rcc_enable_gpiob_clock();

    // Configure Chip Select pin
    cs_pin.port = GPIOB;
    cs_pin.pin = 7;
    cs_pin.mode = GPIO_MODE_OUTPUT;
    cs_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    cs_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    cs_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    gpio_configure_pin(cs_pin);
    gpio_write(cs_pin.port, cs_pin.pin, 0);

    // Configure SPI interface
    spi1.clock_mode = SPI_CLOCK_MODE_0;
    spi1.mode = SPI_MODE_MASTER;
    spi1.baud_rate_prescaler = SPI_BAUD_RATE_PRESCALER_4;
    spi1.significant_bit = SPI_SIGNIFICANT_BIT_MSB;
    spi1.com_mode =  SPI_COM_MODE_SIMPLEX;
    spi1.data_size = SPI_DATA_SIZE_8BIT;
    g_spi_handle = spi_open(spi1);

    // Zero out display buffer
    memset(display_buffer, 0, display_buffer_size);
}

void display_render()
{
    // set Chip Select to high
    gpio_write(GPIOB, 7, 1);

    // Toggle vcom
    vcom = (vcom) ? 0 : 1;

    uint8_t update_data_msg = (vcom<<6) | 0x80;
    spi_write(g_spi_handle, &update_data_msg, 1);

    for (uint32_t line = 0; line < DISPLAY_HEIGHT; line++) {
        uint8_t msg[(DISPLAY_WIDTH/8)+2];

        // Write the line address
        msg[0] = reverse_byte(line + 1);

        // Copy over buffer data
        memcpy(&msg[1], &display_buffer[(line)*(DISPLAY_WIDTH/8)], DISPLAY_WIDTH/8);

        // Write end byte
        msg[(DISPLAY_WIDTH/8)+1] = 0;

        // Send it
        spi_write(g_spi_handle, msg, (DISPLAY_WIDTH/8)+2);
    }
    
    // set Chip select to low
    gpio_write(GPIOB, 7, 0);
}

void display_draw_pixel(uint32_t x, uint32_t y, uint8_t value)
{
    if (x > DISPLAY_WIDTH || y > DISPLAY_HEIGHT) {
        debug_print("x:%u or y:%u exceeds display width:%u or height:%u\r\n", x, y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        return;
    }
    
    uint32_t byte = ((y * DISPLAY_WIDTH) + (x)) / 8;
    uint8_t bit = 7 - (x % 8);

    if (g_draw_attr == DISPLAY_DRAW_ATTR_NORMAL) {
        if (value) {
            display_buffer[byte] |= 1 << bit;
        } else {
            display_buffer[byte] &= ~(1 << bit);
        }
    }

    if (g_draw_attr == DISPLAY_DRAW_ATTR_INVERT) {
        if (value) {
            display_buffer[byte] &= ~(1 << bit);
        } else {
            display_buffer[byte] |= 1 << bit;
        }
    }
}

void display_draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t *bitmap)
{
    if (x > DISPLAY_WIDTH || y > DISPLAY_HEIGHT) {
        debug_print("x:%u or y:%u exceeds display width:%u or height:%u\r\n", x, y, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        return;
    }

    if (x + width > DISPLAY_WIDTH || y + height > DISPLAY_HEIGHT) {
        debug_print("bitmap exceeds screen bounds\r\n");
        return;
    }

    // TODO: Optimize this
    for (uint32_t y_1 = 0; y_1 < height; y_1++) {
        for (uint32_t x_1 = 0; x_1 < width; x_1++) {
            display_draw_pixel(x + x_1, y + y_1, (bitmap[y_1] & (1 << (8-x_1))));
        }
    }
}

void display_clear()
{
    memset(display_buffer, 0, display_buffer_size);
}

void display_set_draw_attr(display_draw_attr_t attr)
{
    g_draw_attr = attr;
}
