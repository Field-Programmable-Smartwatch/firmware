#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

void system_call_handler(uint32_t system_call, uint32_t *system_call_args);

#endif
