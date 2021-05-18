#include <stdint.h>
#include <terminal.h>
#include <libraries/string.h>
#include <kernel/event/event_handler.h>
#include <kernel/task/task_manager.h>
#include <kernel/debug/log.h>
#include <drivers/display/ls013b7h05.h>
#include <applications/menu/menu.h>
#include <key_map.h>

#define MENU_ITEM_MAX 4

uint32_t g_menu_selection = 0;
menu_item_t g_menu_items[MENU_ITEM_MAX];

static void draw_menu()
{
    for (uint32_t i = 0; i < MENU_ITEM_MAX; i++) {
        if (g_menu_selection == i) {
            ls013b7h05_set_draw_attr(DISPLAY_DRAW_ATTR_INVERT);
        }
        terminal_print_at(0, i, g_menu_items[i].name);
        ls013b7h05_set_draw_attr(DISPLAY_DRAW_ATTR_NORMAL);
    }
}

void menu_set_menu_items()
{
    task_manager_t *task_manager = task_manager_get();
    if (!task_manager) {
        log_error(ERROR_INVALID, "Failed to get task_manager");
        return;
    }

    uint32_t menu_item_index = 0;
    for (uint32_t i = 0; i < task_manager->task_count; i++) {
        task_t *task = &task_manager->task_list[i];
        if (string_is_equal(task->name, string("Menu"))) {
            continue;
        }

        if (menu_item_index == MENU_ITEM_MAX) {
            log_error(ERROR_INVALID, "exceeded menu_item buffer");
            break;
        }

        g_menu_items[menu_item_index].name = string_init(g_menu_items[menu_item_index].name_data, 0,
                                                         MENU_ITEM_NAME_MAX);
        string_copy(&g_menu_items[menu_item_index].name, task->name);
        menu_item_index++;
    }
}

void menu_application_start()
{
    task_t *menu_task = task_manager_get_task_by_name(string("Menu"));
    event_queue_t event_queue;
    menu_set_menu_items();
    ls013b7h05_clear();
    draw_menu();
    ls013b7h05_render();
    while (menu_task->status == TASK_STATUS_RUNNING) {
        bool selection_changed = false;
        event_queue = event_handler_poll();
        for (uint8_t i = 0; i < event_queue.length; i++) {
            event_t event = event_queue.events[i];
            if (event.type == EVENT_TYPE_POS_EDGE) {
                if (event.id == ID_BUTTON_UP) {
                    if (g_menu_selection == 0) {
                        continue;
                    }

                    g_menu_selection--;
                    selection_changed = true;
                }

                if (event.id == ID_BUTTON_SELECT) {
                    menu_task->status = TASK_STATUS_STOP;
                    task_manager_start_task_by_name(g_menu_items[g_menu_selection].name);
                }

                if (event.id == ID_BUTTON_DOWN) {
                    if (g_menu_selection == MENU_ITEM_MAX - 1) {
                        continue;
                    }

                    g_menu_selection++;
                    selection_changed = true;
                }
            }
        }
        if (selection_changed) {
            draw_menu();
            ls013b7h05_render();
        }
    }
}
