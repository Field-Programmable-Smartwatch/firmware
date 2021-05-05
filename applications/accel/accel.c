#include <accel.h>
#include <error.h>
#include <log.h>
#include <string.h>
#include <task_manager.h>
#include <event_handler.h>
#include <accelerometer.h>
#include <systick_timer.h>


void accel_application_start()
{
    error_t error;
    uint32_t accelerometer_handle;
    event_queue_t event_queue;
    uint32_t x, y, z;
    task_t *task = task_manager_get_task_by_name(string("accel"));


    error = accelerometer_open(&accelerometer_handle);
    if (error) {
        log_error(error, "Failed to open accelerometer device");
        task->status = TASK_STATUS_STOP;
    }

    while (task->status == TASK_STATUS_RUNNING) {
        event_queue = event_handler_poll();
        for (uint32_t i = 0; i < event_queue.length; i++) {
            event_t event = event_queue.events[i];
            // If any button is pressed, exit out
            if (event.type == EVENT_TYPE_POS_EDGE) {
                break;;
            }
        }

        error = accelerometer_read(accelerometer_handle, &x, &y, &z);
        if (error) {
            log_error(error, "Failed to read from accelerometer");
            break;
        }

        log(LOG_LEVEL_INFO, "fx:%u y:%u z:%u\r\n", x, y, z);
    }

    task->status = TASK_STATUS_STOP;
    task_manager_start_task_by_name(string("Menu"));
}
