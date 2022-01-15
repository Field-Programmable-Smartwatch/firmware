#ifndef INPUT_EVENT_H
#define INPUT_EVENT_H

#include <stdbool.h>
#include <stdint.h>

typedef struct input_event {
    bool valid;
    uint32_t type;
    uint32_t x;
    uint32_t y;
} input_event_t;

#endif
