#include <libraries/error.h>
#include <libraries/input.h>
#include <libraries/system.h>

error_t input_poll(input_event_t *event)
{
    if (!event) {
        return ERROR_INVALID;
    }

    system_call(SYSTEM_CALL_GET_INPUT_EVENT, event);

    return SUCCESS;
}
