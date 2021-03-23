#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <error.h>

typedef enum display_draw_attr{
    DISPLAY_DRAW_ATTR_NORMAL,
    DISPLAY_DRAW_ATTR_INVERT
} display_draw_attr_t;

error_t display_init();
error_t display_render();
error_t display_draw_pixel(uint32_t x, uint32_t y, uint8_t value);
error_t display_draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t *bitmap);
void display_clear();
void display_set_draw_attr(display_draw_attr_t attr);

#endif
