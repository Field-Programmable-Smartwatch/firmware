#ifndef ERROR_H
#define ERROR_H

typedef enum {
    SUCCESS,
    ERROR_INVALID,
    ERROR_NO_MEMORY,
    ERROR_IO,
    ERROR_TIMEOUT,
} error_t;

struct error_message {
    error_t error_id;
    const char *message;
};

#endif
