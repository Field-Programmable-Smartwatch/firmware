#include <stm32wb55xx.h>
#include <time_app.h>
#include <time.h>
#include <event_handler.h>
#include <stdint.h>
#include <stdbool.h>
#include <display.h>
#include <terminal.h>
#include <debug.h>
#include <menu.h>
#include <task_manager.h>
#include <string.h>
#include <rtc.h>

char *weekdays[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
char *months[] = {"January", "February", "March", "April", "May", "June",
                  "July", "August", "September", "October", "November", "December"};

void draw_time(datetime_t *datetime)
{
    terminal_print_at(6, 3, "%s", weekdays[datetime->weekday]);
    terminal_print_at(5, 4, "%s %u", months[datetime->month], datetime->day);
    terminal_print_at(5, 5, "%02u:%02u.%02u", datetime->hours, datetime->minutes, datetime->seconds);
}

void change_time()
{
    task_t *task = task_manager_get_task_by_name("Set Time");
    uint8_t select_button_count = 0;
    event_queue_t event_queue;
    bool time_changed;
    datetime_t datetime;

    rtc_get_date_and_time(&datetime);
    display_clear();
    draw_time(&datetime);
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
                        if (datetime.hours == 24) {
                            datetime.hours = 0;
                        } else {
                            datetime.hours++;
                        }
                    } else if (select_button_count == 1) {
                        if (datetime.minutes == 59) {
                            datetime.minutes = 0;
                        } else {
                            datetime.minutes++;
                        }
                    } else if (select_button_count == 2) {
                        if (datetime.seconds == 59) {
                            datetime.seconds = 0;
                        } else {
                            datetime.seconds++;
                        }
                    }
                }

                if (event.id == ID_BUTTON_SELECT) {
                    if (select_button_count == 2) {
                        rtc_set_date_and_time(&datetime);
                        task->status = TASK_STATUS_STOP;
                        task_manager_start_task_by_name("Time");
                    }
                    select_button_count++;
                }

                if (event.id == ID_BUTTON_DOWN) {
                    if (select_button_count == 0) {
                        if (datetime.hours == 0) {
                            datetime.hours = 24;
                        } else {
                            datetime.hours--;
                        }
                    } else if (select_button_count == 1) {
                        if (datetime.minutes == 0) {
                            datetime.minutes = 59;
                        } else {
                            datetime.minutes--;
                        }
                    } else if (select_button_count == 2) {
                        if (datetime.seconds == 0) {
                            datetime.seconds = 59;
                        } else {
                            datetime.seconds--;
                        }
                    }
                }
            }            
        }

        if (time_changed) {
            draw_time(&datetime);
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
    datetime_t datetime;
    rtc_get_date_and_time(&datetime);
    display_clear();
    draw_time(&datetime);
    display_render();
    while (task->status == TASK_STATUS_RUNNING) {
        asm("wfi");
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
        rtc_get_date_and_time(&datetime);
        draw_time(&datetime);
        display_render();
    }
    
}
