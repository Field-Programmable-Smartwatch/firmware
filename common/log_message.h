#ifndef LOG_MESSAGE_H
#define LOG_MESSAGE_H

#include <libraries/string.h>

#define LOG_MSG_MAX_LENGTH 512



typedef struct log_message {
    error_t error;
    uint32_t level;
    char message_data[LOG_MSG_MAX_LENGTH];
    string_t message;
} log_message_t;

#endif
