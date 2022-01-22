#include <libraries/system.h>
#include <libraries/asm.h>

void system_call(system_call_t system_call, void *args)
{
    register uint32_t r0 asm("r0") = (uint32_t)args;
    switch (system_call) {
    case SYSTEM_CALL_START_FIRST_TASK:
        asm_svc(SYSTEM_CALL_START_FIRST_TASK, r0);
        break;

    case SYSTEM_CALL_LOG:
        asm_svc(SYSTEM_CALL_LOG, r0);
        break;

    case SYSTEM_CALL_GET_INPUT_EVENT:
        asm_svc(SYSTEM_CALL_GET_INPUT_EVENT, r0);
        break;

    case SYSTEM_CALL_RENDER:
        asm_svc(SYSTEM_CALL_RENDER, r0);
        break;
    }
}
