#include <stm32wb55xx.h>
#include <stdint.h>
#include <string.h>

#include <gpio.h>
#include <debug.h>
#include <spi.h>

#define WIDTH 144
#define HEIGHT 168

uint8_t vcom = 0;
uint32_t display_buffer_size = (WIDTH * HEIGHT) / 8;
uint8_t display_buffer[(WIDTH * HEIGHT) / 8];

void set_display_buffer(uint8_t value)
{
    memset(display_buffer, value, display_buffer_size);
}

void display_init()
{
    // set NSS to low
    gpio_write(GPIOB, 7, 0);

    set_display_buffer(0);
}

void display_render()
{
    // set NSS to high
    gpio_write(GPIOB, 7, 1);

    // Toggle vcom
    vcom = (vcom) ? 0 : 1;

    uint8_t update_data_msg = vcom | 1;
    spi_transmit(SPI1, update_data_msg);

    for (uint32_t line = 0; line < HEIGHT; line++) {
        uint8_t msg[(WIDTH/8)+2];

        // Write the line address
        msg[0] = line + 1;

        // Copy over buffer data
        //memset(&msg[1], value, WIDTH/8);
        memcpy(&msg[1], &display_buffer[(line)*(WIDTH/8)], WIDTH/8);

        // Write end byte
        msg[(WIDTH/8)+1] = 0;

        // Send it
        spi_transmit_buffer(SPI1, msg, (WIDTH/8)+2);
    }
    
    // set NSS to low
    gpio_write(GPIOB, 7, 0);
}

void configure()
{
    debug_init();
    
    // Enable spi clock
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    gpio_configuration_t cs_pin;
    spi_configuration_t spi1;

    cs_pin.port = GPIOB;
    cs_pin.pin = 7;
    cs_pin.mode = GPIO_MODE_OUTPUT;
    cs_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    cs_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    cs_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    gpio_configure_pin(cs_pin);

    spi1.clock_mode = SPI_CLOCK_MODE_0;
    spi1.mode = SPI_MODE_MASTER;
    spi1.baud_rate_prescaler = SPI_BAUD_RATE_PRESCALER_4;
    spi1.significant_bit = SPI_SIGNIFICANT_BIT_LSB;
    spi1.com_mode =  SPI_COM_MODE_SIMPLEX;
    spi1.data_size = SPI_DATA_SIZE_8BIT;
    spi_configure(SPI1, spi1);

    display_init();
}

void display_clear()
{
    uint8_t msg = 0b00000110;
    gpio_write(GPIOB, 7, 1);
    spi_transmit(SPI1, msg);
    gpio_write(GPIOB, 7, 0);    
}

void main()
{
    configure();
    display_clear();
    while (1) {
        set_display_buffer(0x00);
        display_render();
        for (uint32_t i = 0; i < 800000; i++);        
        set_display_buffer(0xff);
        display_render();
        for (uint32_t i = 0; i < 800000; i++);
    }
}
