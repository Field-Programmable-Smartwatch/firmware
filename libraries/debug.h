#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>

#define DEBUG_MSG_MAX_LENGTH 256
void debug_init();
void debug_print(char *format, ...);
uint8_t debug_wait_for_input();
#endif
