#include <stdint.h>
#include <bluefruit.h>
#include <spi.h>
#include <gpio.h>
#include <systick_timer.h>
#include <sdep.h>
#include <rcc.h>
#include <debug.h>
#include <string.h>

#define BLUEFRUIT_CS_PORT GPIOA
#define BLUEFRUIT_CS_PIN 15

#define BLUEFRUIT_IRQ_PORT GPIOA
#define BLUEFRUIT_IRQ_PIN 14

static int32_t g_spi_handle = -1;;

static inline void bluefruit_select()
{
    gpio_write(BLUEFRUIT_CS_PORT, BLUEFRUIT_CS_PIN, 0);
}


static inline void bluefruit_deselect()
{
    gpio_write(BLUEFRUIT_CS_PORT, BLUEFRUIT_CS_PIN, 1);
}

static void bluefruit_send(sdep_command_t command, void *payload, uint32_t size)
{
    sdep_header_t packet;
    packet.message_type = SDEP_MESSAGE_TYPE_COMMAND;
    packet.command = command;
    packet.payload_size = size;
    /* debug_print("sending packet:\r\n type: %x\r\n command: %x\r\n length: %u\r\n", packet.message_type, packet.command, size); */

    bluefruit_select();
    spi_write(g_spi_handle, &packet, sizeof(sdep_header_t));
    if (size && payload) {
        spi_write(g_spi_handle, payload, size);
    }
    bluefruit_deselect();
}

static void bluefruit_receive(sdep_header_t *packet_header, void *payload)
{
    if (!packet_header || !payload) {
        debug_print("bluefruit_receive: NULL buffers\r\n");
        return;
    }
    
    uint32_t count = 0;
    // Check if IRQ pin is raised and a packet is ready to be received
    while (!gpio_read(BLUEFRUIT_IRQ_PORT, BLUEFRUIT_IRQ_PIN) && count++ < 50);
    if (count == 50) {
        debug_print("bluefruit_read: no data to be received\r\n");
        return;
    }
    
    bluefruit_select();
    
    count = 0;

    do {
        // Find the packet header
        spi_read_write(g_spi_handle, &packet_header->message_type, 0xff, 1);
        if (packet_header->message_type == 0xff) {
            bluefruit_deselect();
            systick_timer_wait_ms(50);
            bluefruit_select();
        }
    } while (packet_header->message_type != SDEP_MESSAGE_TYPE_RESPONSE &&
             packet_header->message_type != SDEP_MESSAGE_TYPE_ERROR &&
             count++ < 50);

    if (count == 50) {
        debug_print("bluefruit_read: failed to read packet\r\n");
        return;
    }

    // Receive the rest of the packet header
    spi_read_write(g_spi_handle, (&packet_header->message_type)+1, 0xff, 3);

    if (packet_header->payload_size > 16) {
        debug_print("bluefruit_receive: payload size too big\r\n");
        return;
    }

    /* debug_print("\r\nRecv packet:\r\n type: %x\r\n cmd: %x\r\n size: %u\r\n", */
    /*             packet_header->message_type,packet_header->command,packet_header->payload_size); */

    // Receive the payload
    spi_read_write(g_spi_handle, payload, 0xff, packet_header->payload_size);

    bluefruit_deselect();
}

void bluefruit_read(void *buffer, uint32_t size)
{
    uint8_t *buffer_ptr = buffer;
    sdep_header_t packet_header;
    uint8_t payload[16];
    memset(&packet_header, 0, sizeof(sdep_header_t));

    do {
        bluefruit_receive(&packet_header, payload);
        uint32_t copy_size = (size < packet_header.payload_size) ? size : packet_header.payload_size;
        size -= copy_size;
        memcpy(buffer_ptr, payload, copy_size);
        buffer_ptr += copy_size;
        // Need to put in this wait, looks like packets get corrupted if not. Maybe clock too fast?
        systick_timer_wait_ms(50);
    } while (packet_header.more_data);
}

void bluefruit_write(void *buffer, uint32_t size)
{
    bluefruit_send(SDEP_COMMAND_ATWRAPPER, buffer, size);
}

void bluefruit_init()
{
    gpio_configuration_t cs_pin;
    gpio_configuration_t irq_pin;
    spi_configuration_t spi;

    rcc_enable_gpioa_clock();

    // Configure Chip Select pin
    cs_pin.port = BLUEFRUIT_CS_PORT;
    cs_pin.pin = BLUEFRUIT_CS_PIN;
    cs_pin.mode = GPIO_MODE_OUTPUT;
    cs_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    cs_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    cs_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    gpio_configure_pin(cs_pin);
    gpio_write(cs_pin.port, cs_pin.pin, 1);

    // Configure IRQ pin
    irq_pin.port = BLUEFRUIT_IRQ_PORT;
    irq_pin.pin = BLUEFRUIT_IRQ_PIN;
    irq_pin.mode = GPIO_MODE_INPUT;
    irq_pin.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    irq_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    irq_pin.pull_resistor = GPIO_PULL_RESISTOR_DOWN;
    gpio_configure_pin(irq_pin);

    // Configure SPI interface
    spi.clock_mode = SPI_CLOCK_MODE_0;
    spi.mode = SPI_MODE_MASTER;
    spi.baud_rate_prescaler = SPI_BAUD_RATE_PRESCALER_8;
    spi.significant_bit = SPI_SIGNIFICANT_BIT_MSB;
    spi.com_mode =  SPI_COM_MODE_FULL_DUPLEX;
    spi.data_size = SPI_DATA_SIZE_8BIT;
    g_spi_handle = spi_open(spi);

    // Reset Bluefruit Device
    bluefruit_send(SDEP_COMMAND_INITIALIZE, 0, 0);

    // Wait one second to reboot Bluefruit
    systick_timer_wait_ms(1000);
}
