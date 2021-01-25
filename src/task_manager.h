#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <stdint.h>

#define TASK_LIST_MAX 16
#define TASK_NAME_MAX 16

typedef enum task_status {
    TASK_STATUS_STOPPED,
    TASK_STATUS_STOP,
    TASK_STATUS_RUNNING,
    TASK_STATUS_START
} task_status_t;

typedef struct task {
    task_status_t status;
    char name[TASK_NAME_MAX];
    void (*task_start)(void);
} task_t;

typedef struct task_manager{
    uint32_t task_count;
    task_t task_list[TASK_LIST_MAX];
} task_manager_t;


task_t *task_manager_get_task_by_name(char *name);
uint32_t task_manager_start_task_by_name(char *name);
void task_manager_add_task(char *task_name, void (*task_start)(void));
task_manager_t *task_manager_get();
void task_manager_init();
void task_manager_start();

#endif
