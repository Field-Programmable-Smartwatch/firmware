#ifndef LOG_H
#define LOG_H

#include <common/log_message.h>

typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
} log_level_t;

void log(log_level_t log_level, char *format, ...);
void log_error(error_t error, char *format, ...);
void log_info(char *format, ...);
void log_debug(char *format, ...);

#endif
