#include <stdint.h>
#include <string.h>
#include <event_handler.h>
#include <display.h>
#include <terminal.h>
#include <debug.h>
#include <menu.h>
#include <time.h>

#define MENU_ITEM_MAX 3

extern uint32_t *_bootloader_magic[];

uint32_t g_menu_selection = 0;
menu_item_t g_menu_items[MENU_ITEM_MAX];

static void draw_menu()
{
    for (uint32_t i = 0; i < MENU_ITEM_MAX; i++) {
        if (g_menu_selection == i) {
            display_set_draw_attr(DISPLAY_DRAW_ATTR_INVERT);
        }
        terminal_print_at(0, i, g_menu_items[i].name);
        display_set_draw_attr(DISPLAY_DRAW_ATTR_NORMAL);
    }
}

void bootloader_reboot()
{
    debug_print("LOADING BOOTLOADER!\r\n");
    display_clear();
    terminal_print_at(0, 4, "LOADING BOOTLOADER");
    display_render();
    *((unsigned long *)_bootloader_magic) = 0x10ADB007;
    NVIC_SystemReset();
}

void menu_set_menu_items()
{
    strncpy(g_menu_items[0].name, "Time", 16);
    g_menu_items[0].app_callback = &time_application_start;

    strncpy(g_menu_items[1].name, "Set Time", 16);
    g_menu_items[1].app_callback = &change_time;

    strncpy(g_menu_items[2].name, "Bootloader", 16);
    g_menu_items[2].app_callback = &bootloader_reboot;
}

void menu_application_start()
{
    event_queue_t event_queue;
    menu_set_menu_items();
    display_clear();
    draw_menu();
    display_render();
    while (1) {
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
                    g_menu_items[g_menu_selection].app_callback();
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
            display_render();
        }
    }
}
