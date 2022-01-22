#include <libraries/log.h>
#include <libraries/input.h>
#include <libraries/string.h>
#include <libraries/graphics.h>

extern uint32_t _sidata[];
extern uint32_t _sdata[];
extern uint32_t _edata[];
extern uint32_t _sbss[];
extern uint32_t _ebss[];

static bool in_rect(uint32_t x, uint32_t y, rectangle_t rect)
{
    if (x < rect.x || x > rect.x + rect.width ||
        y < rect.y || y > rect.y + rect.height) {
        return false;
    }

    return true;
}

__attribute__((section(".text.entry")))
void main()
{
    // Copy data section from flash memory to ram
    uint32_t data_section_size = _edata - _sdata;
    memcpy(_sdata, _sidata, data_section_size*4);

    // Zero out bss
    uint32_t bss_section_size = _ebss - _sbss;
    memset(_sbss, 0, bss_section_size*4);

    error_t error;
    rectangle_t rect1 = (rectangle_t){.x = 0, .y = 0, .width = 100, .height = 100};
    bool rect1_touched = false;
    rectangle_t rect2 = (rectangle_t){.x = 140, .y = 0, .width = 100, .height = 100};
    bool rect2_touched = false;
    rectangle_t rect3 = (rectangle_t){.x = 0, .y = 140, .width = 100, .height = 100};
    bool rect3_touched = false;
    rectangle_t rect4 = (rectangle_t){.x = 140, .y = 140, .width = 100, .height = 100};
    bool rect4_touched = false;

    graphics_init(240, 240);
    graphics_draw_rectangle(rect1, COLOR_BLUE);
    graphics_draw_rectangle(rect2, COLOR_BLUE);
    graphics_draw_rectangle(rect3, COLOR_BLUE);
    graphics_draw_rectangle(rect4, COLOR_BLUE);

    while(1) {
        input_event_t event = {0};
        error = input_poll(&event);
        if (error || !event.valid) {
            continue;
        }
        log_info("Event received: type:%u x:%u y:%u", event.type, event.x, event.y);

        if ((event.type == 0 || event.type == 2)) {
            if (in_rect(event.x, event.y, rect1) && !rect1_touched) {
                graphics_draw_rectangle(rect1, COLOR_RED);
                rect1_touched = true;
            } else if (in_rect(event.x, event.y, rect2) && !rect1_touched) {
                graphics_draw_rectangle(rect2, COLOR_RED);
                rect2_touched = true;
            } else if (in_rect(event.x, event.y, rect3) && !rect1_touched) {
                graphics_draw_rectangle(rect3, COLOR_RED);
                rect3_touched = true;
            } else if (in_rect(event.x, event.y, rect4) && !rect1_touched) {
                graphics_draw_rectangle(rect4, COLOR_RED);
                rect4_touched = true;
            }
        } else if (event.type == 1) {
            if (rect1_touched) {
                graphics_draw_rectangle(rect1, COLOR_BLUE);
                rect1_touched = false;
            } else if (rect2_touched) {
                graphics_draw_rectangle(rect2, COLOR_BLUE);
                rect2_touched = false;
            } else if (rect3_touched) {
                graphics_draw_rectangle(rect3, COLOR_BLUE);
                rect3_touched = false;
            } else if (rect4_touched) {
                graphics_draw_rectangle(rect4, COLOR_BLUE);
                rect4_touched = false;
            }
        }
    }
}
