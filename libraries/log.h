#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <error.h>

#define LOG_MSG_MAX_LENGTH 512

typedef enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
} log_level_t;

#define log_error(error, format, ...) do {                              \
        log(LOG_LEVEL_ERROR, "ERROR %s [%s]: ", __func__, error_get_message(error)); \
        log(LOG_LEVEL_ERROR, format, ##__VA_ARGS__);                    \
        log(LOG_LEVEL_ERROR, "\r\n");                                   \
    } while (0)

#define log_info(format, ...) do {                                  \
        log(LOG_LEVEL_INFO, "INFO %s: ", __func__);                 \
        log(LOG_LEVEL_INFO, format, ##__VA_ARGS__);                 \
        log(LOG_LEVEL_INFO, "\r\n");                                \
    } while (0)

#define log_debug(format, ...) do {                                     \
        log(LOG_LEVEL_DEBUG, "DEBUG %s: ", __func__);                   \
        log(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__);                    \
        log(LOG_LEVEL_DEBUG, "\r\n");                                   \
    } while (0)


void log(log_level_t log_level, char *format, ...);
uint8_t log_wait_for_input();
void log_init();

#endif



