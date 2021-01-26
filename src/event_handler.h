#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <stm32wb55xx.h>
#include <stdint.h>
#include <stdbool.h>

#define ID_BUTTON_UP 0
#define ID_BUTTON_SELECT 1
#define ID_BUTTON_DOWN 2
#define ID_BUTTON_LOAD_BOOTLOADER 3

#define EVENT_TYPE_NULL 0
#define EVENT_TYPE_POS_EDGE 1
#define EVENT_TYPE_NEG_EDGE 2

#define EVENT_QUEUE_LENGTH 8


typedef struct event {
    uint8_t id;
    uint8_t type;
} event_t;

typedef struct event_queue {
    uint32_t length;
    event_t events[EVENT_QUEUE_LENGTH];
} event_queue_t;

typedef struct event_source {
    bool active;
    uint8_t id;
    GPIO_TypeDef *gpio_port;
    uint8_t gpio_pin;
    bool masked;
    uint8_t input_value;
    uint32_t unmask_tick;
} event_source_t;

void event_handler_init();
event_queue_t event_handler_poll();
void event_handler_clear_event_queue();
#endif
