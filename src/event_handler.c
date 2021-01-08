#include <stm32wb55xx.h>
#include <string.h>
#include <stdint.h>
#include <gpio.h>
#include <event_handler.h>
#include <systick_timer.h>
#include <debug.h>

#define EVENT_SOURCE_LENGTH 4
#define DEBOUNCE_THRESHOLD 5 // In millisec

event_source_t source[EVENT_SOURCE_LENGTH];

static void event_source_init(event_source_t *event_source, uint32_t id, GPIO_TypeDef *gpio_port, uint8_t gpio_pin)
{
    if (!event_source) {
        debug_print("ERROR: event_source_add: passed NULL event_source\r\n");
        return;
    }

    memset(event_source, 0, sizeof(event_source_t));
    event_source->id = id;
    event_source->gpio_port = gpio_port;
    event_source->gpio_pin = gpio_pin;
    event_source->input_value = gpio_read(gpio_port, gpio_pin);
}

void event_handler_init()
{
    gpio_configuration_t gpio_pb6;
    gpio_configuration_t gpio_pb5;
    gpio_configuration_t gpio_pb4;
    gpio_configuration_t gpio_pb3;

    gpio_pb6.port = GPIOB;
    gpio_pb6.pin = 6;
    gpio_pb6.mode = GPIO_MODE_INPUT;
    gpio_pb6.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_pb6.output_speed = GPIO_OUTPUT_SPEED_LOW;
    gpio_pb6.pull_resistor = GPIO_PULL_RESISTOR_UP;
    gpio_configure_pin(gpio_pb6);
    event_source_init(&source[0], ID_BUTTON_UP, GPIOB, 6);

    gpio_pb5.port = GPIOB;
    gpio_pb5.pin = 5;
    gpio_pb5.mode = GPIO_MODE_INPUT;
    gpio_pb5.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_pb5.output_speed = GPIO_OUTPUT_SPEED_LOW;
    gpio_pb5.pull_resistor = GPIO_PULL_RESISTOR_UP;
    gpio_configure_pin(gpio_pb5);
    event_source_init(&source[1], ID_BUTTON_SELECT, GPIOB, 5);

    gpio_pb4.port = GPIOB;
    gpio_pb4.pin = 4;
    gpio_pb4.mode = GPIO_MODE_INPUT;
    gpio_pb4.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_pb4.output_speed = GPIO_OUTPUT_SPEED_LOW;
    gpio_pb4.pull_resistor = GPIO_PULL_RESISTOR_UP;
    gpio_configure_pin(gpio_pb4);
    event_source_init(&source[2], ID_BUTTON_DOWN, GPIOB, 4);

    gpio_pb3.port = GPIOB;
    gpio_pb3.pin = 3;
    gpio_pb3.mode = GPIO_MODE_INPUT;
    gpio_pb3.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_pb3.output_speed = GPIO_OUTPUT_SPEED_LOW;
    gpio_pb3.pull_resistor = GPIO_PULL_RESISTOR_UP;
    gpio_configure_pin(gpio_pb3);
    event_source_init(&source[3], ID_BUTTON_LOAD_BOOTLOADER, GPIOB, 3);

}

static uint8_t event_source_read_input(event_source_t *event_source)
{
    return gpio_read(event_source->gpio_port, event_source->gpio_pin);
}

static void event_queue_add_event(event_queue_t *event_queue, event_t event)
{
    if (event_queue->length >= EVENT_QUEUE_LENGTH) {
        return;
    }

    event_queue->events[event_queue->length++] = event;
}

event_queue_t event_handler_poll()
{
    event_queue_t event_queue;
    event_queue.length = 0;
    for (uint32_t i = 0; i < EVENT_SOURCE_LENGTH; i++) {
        uint8_t current_input = event_source_read_input(&source[i]);
        if (current_input != source[i].input_value) {
            source[i].input_value = current_input;
            if (!source[i].masked) {
                source[i].masked = true;
            }
            source[i].unmask_tick = systick_timer_get_tick_count() + DEBOUNCE_THRESHOLD;
            continue;
        }

        if (source[i].masked &&
            systick_timer_get_tick_count() >= source[i].unmask_tick) {
            event_t event;
            source[i].masked = false;
            event.id = source[i].id;
            if (source[i].input_value) {
                event.type = EVENT_TYPE_POS_EDGE;
            } else {
                event.type = EVENT_TYPE_NEG_EDGE;
            }

            event_queue_add_event(&event_queue, event);
        }
    }
    return event_queue;
}
