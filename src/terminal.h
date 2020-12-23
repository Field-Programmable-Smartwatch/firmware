#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>

typedef struct terminal_cursor {
    uint32_t x;
    uint32_t y;
} terminal_cursor_t;

struct psf_header {
    uint32_t magic;
    uint32_t version;
    uint32_t offset;
    uint32_t flags;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
};

typedef struct terminal_configuration {
    terminal_cursor_t cursor;
    uint32_t width;
    uint32_t height;
    struct psf_header *font_header;
    uint8_t *font_bitmap;
} terminal_configuration_t;

void terminal_init(terminal_configuration_t config);
void terminal_print(char *format, ...);
void terminal_set_cursor(uint32_t x, uint32_t y);

#endif
