#ifndef INPUT_TASK_H
#define INPUT_TASK_H

#include <libraries/error.h>
#include <common/input_event.h>

error_t input_task_init();
error_t input_task_get_event(input_event_t *event);
void input_task_entry();

#endif
