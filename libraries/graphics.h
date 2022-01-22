#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <libraries/error.h>
#include <libraries/color.h>
#include <common/rectangle.h>

error_t graphics_init(uint32_t display_width, uint32_t display_height);
error_t graphics_draw_rectangle(rectangle_t rect, color16_t color);

#endif
