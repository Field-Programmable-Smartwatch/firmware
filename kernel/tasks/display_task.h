#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

#include <libraries/error.h>
#include <common/render_request.h>

error_t display_task_init();
void display_task_entry();
error_t display_task_queue_request(render_request_t *request);

#endif
