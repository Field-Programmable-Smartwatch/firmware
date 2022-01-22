#include <kernel/system.h>
#include <kernel/tasks/input_task.h>
#include <kernel/tasks/display_task.h>
#include <kernel/debug/log.h>
#include <common/log_message.h>
#include <common/render_request.h>

static void system_call_handle_log(log_message_t *message)
{
    if (!message) {
        return;
    }

    switch (message->level) {
        case LOG_LEVEL_ERROR:
            log_error(message->error, message->message_data);
            break;

        case LOG_LEVEL_INFO:
            log_info(message->message_data);
            break;

        case LOG_LEVEL_DEBUG:
            log_debug(message->message_data);
            break;
    }
}

void system_call_handler(uint32_t system_call, uint32_t *system_call_args)
{
    switch (system_call) {
    case 1:
        system_call_handle_log((log_message_t *)system_call_args[0]);
        break;

    case 2:
        input_task_get_event((input_event_t *)system_call_args[0]);
        break;

    case 3:
        display_task_queue_request((render_request_t *)system_call_args[0]);
        break;
    }
}
