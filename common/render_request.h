#ifndef RENDER_REQUEST_H
#define RENDER_REQUEST_H

#include <stdbool.h>
#include <stdint.h>
#include <common/rectangle.h>

typedef struct render_request {
    bool *render_complete;
    rectangle_t rect;
} render_request_t;
#endif
