#include <stm32wb55xx.h>
#include <task_manager.h>
#include <stdint.h>
#include <string.h>
#include <time_app.h>
#include <menu.h>
#include <ATtui.h>
#include <display.h>
#include <terminal.h>
#include <debug.h>

task_manager_t g_task_manager;
extern uint32_t *_bootloader_magic[];

static void bootloader_reboot()
{
    debug_print("LOADING BOOTLOADER!\r\n");
    display_clear();
    terminal_print_at(0, 4, "LOADING BOOTLOADER");
    display_render();
    *((unsigned long *)_bootloader_magic) = 0x10ADB007;
    NVIC_SystemReset();
}

task_t *task_manager_get_task_by_name(char *name)
{
    if (!name) {
        return (task_t *)0;
    }
    task_t *task = 0;
    for (uint32_t i = 0; i < g_task_manager.task_count; i++) {
        if (strncmp(g_task_manager.task_list[i].name, name, TASK_NAME_MAX) == 0) {
            task = &g_task_manager.task_list[i];
            break;
        }
    }

    return task;
}

uint32_t task_manager_start_task_by_name(char *name)
{
    if (!name) {
        return 1;
    }

    task_t *task = task_manager_get_task_by_name(name);
    if (!task) {
        return -1;
    }

    task->status = TASK_STATUS_START;
    return 0;
}

void task_manager_add_task(char *task_name, void (*task_start)(void))
{
    if (g_task_manager.task_count == TASK_LIST_MAX || !task_name) {
        return;
    }

    task_t *task = &g_task_manager.task_list[g_task_manager.task_count];
    strncpy(task->name, task_name, TASK_NAME_MAX);
    task->task_start = task_start;
    g_task_manager.task_count++;
}

task_manager_t *task_manager_get()
{
    return &g_task_manager;
}

void task_manager_init()
{
    memset(&g_task_manager, 0, sizeof(task_manager_t));
    task_manager_add_task("Menu", &menu_application_start);
    task_manager_add_task("Time", &time_application_start);
    task_manager_add_task("Set Time", &change_time);
    task_manager_add_task("Bootloader", &bootloader_reboot);
    task_manager_add_task("ATtui", &ATtui_application_start);
    // TODO: look for additional tasks to add in flash memory

    task_manager_start_task_by_name("Time");
}

void task_manager_start()
{
    while (1) {
        task_t *running_task = 0;
        for (uint32_t i = 0; i < g_task_manager.task_count; i++) {
            task_t *task = &g_task_manager.task_list[i];

            if (task->status == TASK_STATUS_STOP) {
                task->status = TASK_STATUS_STOPPED;
            }

            if (task->status == TASK_STATUS_START) {
                running_task = task;
            }
        }

        if (running_task) {
            running_task->status = TASK_STATUS_RUNNING;
            running_task->task_start();
        }
    }
}
