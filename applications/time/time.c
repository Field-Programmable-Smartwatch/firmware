#include <stm32wb55xx.h>
#include <time.h>
#include <event_handler.h>
#include <stdint.h>
#include <stdbool.h>
#include <display.h>
#include <terminal.h>
#include <debug.h>
#include <systick_timer.h>
#include <menu.h>
#include <task_manager.h>
#include <string.h>

char week_day[16] = "Monday";
char month[16] = "January";
uint8_t day = 1;
uint8_t hours = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;

uint32_t seconds_tick = 0;

void draw_time()
{
    terminal_print_at(6, 3, "%s", week_day);
    terminal_print_at(5, 4, "%s %u", month, day);
    terminal_print_at(5, 5, "%02u:%02u.%02u", hours, minutes, seconds);
}

bool update_time()
{
    bool time_changed = false;
    uint32_t current_tick = systick_timer_get_tick_count();
    if (current_tick - seconds_tick > 1000) {
        seconds_tick = current_tick;
        time_changed = true;
        seconds++;
    }

    if (seconds >= 60) {
        seconds = 0;
        minutes++;
    }

    if (minutes >= 60) {
        minutes = 0;
        hours++;
    }

    if (hours >= 25) {
        hours = 0;
        day++;
    }
    return time_changed;
}

void change_time()
{
    task_t *task = task_manager_get_task_by_name("Set Time");
    uint8_t select_button_count = 0;
    event_queue_t event_queue;
    bool time_changed;

    display_clear();
    draw_time();
    terminal_print_at(0, 0, "Changing hours");
    display_render();
    
    while (task->status == TASK_STATUS_RUNNING) {
        asm("wfi");
        time_changed = false;
        event_queue = event_handler_poll();
        for (uint8_t i = 0; i < event_queue.length; i++) {
            event_t event = event_queue.events[i];
            if (event.type == EVENT_TYPE_POS_EDGE) {
                time_changed = true;
                if (event.id == ID_BUTTON_UP) {
                    if (select_button_count == 0) {
                        if (hours == 24) {
                            hours = 0;
                        } else {
                            hours++;
                        }
                    } else if (select_button_count == 1) {
                        if (minutes == 59) {
                            minutes = 0;
                        } else {
                            minutes++;
                        }
                    } else if (select_button_count == 2) {
                        if (seconds == 59) {
                            seconds = 0;
                        } else {
                            seconds++;
                        }
                    }
                }

                if (event.id == ID_BUTTON_SELECT) {
                    if (select_button_count == 2) {
                        task->status = TASK_STATUS_STOP;
                        task_manager_start_task_by_name("Time");
                    }
                    select_button_count++;
                }

                if (event.id == ID_BUTTON_DOWN) {
                    if (select_button_count == 0) {
                        if (hours == 0) {
                            hours = 24;
                        } else {
                            hours--;
                        }
                    } else if (select_button_count == 1) {
                        if (minutes == 0) {
                            minutes = 59;
                        } else {
                            minutes--;
                        }
                    } else if (select_button_count == 2) {
                        if (seconds == 0) {
                            seconds = 59;
                        } else {
                            seconds--;
                        }
                    }
                }
            }            
        }

        if (time_changed) {
            draw_time();
            if (select_button_count == 0) {
                terminal_print_at(0, 0, "Changing hours");
            }
            if (select_button_count == 1) {
                terminal_print_at(0, 0, "Changing minutes");
            }
            if (select_button_count == 2) {
                terminal_print_at(0, 0, "Changing seconds");
            }
            display_render();
        }
    }
}

void time_application_start()
{
    task_t *task = task_manager_get_task_by_name("Time");
    event_queue_t event_queue;
    display_clear();
    draw_time();
    display_render();
    seconds_tick = systick_timer_get_tick_count();
    while (task->status == TASK_STATUS_RUNNING) {
        asm("wfi");
        bool time_changed = update_time();
        event_queue = event_handler_poll();
        for (uint8_t i = 0; i < event_queue.length; i++) {
            event_t event = event_queue.events[i];
            if (event.type == EVENT_TYPE_POS_EDGE) {
                if (event.id == ID_BUTTON_UP) {
                    task->status = TASK_STATUS_STOP;
                    task_manager_start_task_by_name("Menu");
                }

                if (event.id == ID_BUTTON_SELECT) {
                    task->status = TASK_STATUS_STOP;
                    task_manager_start_task_by_name("Menu");
                }

                if (event.id == ID_BUTTON_DOWN) {
                    task->status = TASK_STATUS_STOP;
                    task_manager_start_task_by_name("Menu");
                }
            }
        }
        if (time_changed) {
            draw_time();
            display_render();
        }
    }
    
}
