#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

typedef enum system_call {
    SYSTEM_CALL_START_FIRST_TASK,
    SYSTEM_CALL_LOG,
    SYSTEM_CALL_GET_INPUT_EVENT,
    SYSTEM_CALL_RENDER,
} system_call_t;

void system_call(system_call_t system_call, void *args);

#endif
