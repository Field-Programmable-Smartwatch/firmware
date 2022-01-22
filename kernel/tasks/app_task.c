#include <stdint.h>
#include <libraries/error.h>
#include <kernel/task/task_manager.h>
#include <kernel/tasks/app_task.h>

error_t app_task_init()
{
    task_handle_t app_handle;
    void (*app_main)(void) = (void (*)(void))0x08010000;
    uint32_t *app_stack = (uint32_t *)0x2002fb00;
    return task_manager_init_task(app_main,
                                  1,
                                  320,
                                  app_stack,
                                  &app_handle);
}
