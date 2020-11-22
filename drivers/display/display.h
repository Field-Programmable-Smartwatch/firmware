#ifndef DISPLAY_H

#include <stdint.h>

void display_init();
void display_render();
void display_draw_pixel(uint32_t x, uint32_t y, uint8_t value);
void display_draw_bitmap(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t *bitmap);
void display_clear();

#endif
