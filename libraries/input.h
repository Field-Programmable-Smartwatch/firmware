#ifndef INPUT_H
#define INPUT_H

#include <libraries/error.h>
#include <common/input_event.h>

error_t input_poll(input_event_t *event);

#endif
