#include <terminal.h>
#include <stdarg.h> 
#include <display.h>
#include <string.h>
#include <log.h>

#define MSG_MAX_LENGTH 256

extern uint8_t _binary_cp850_8x16_psfu_start[];
extern uint8_t _binary_cp850_8x16_psfu_end[];
terminal_configuration_t terminal;

void terminal_init(terminal_configuration_t config)
{
    terminal = config;
    terminal.cursor.x = 0;
    terminal.cursor.y = 0;
    terminal.font_header = (struct psf_header *)_binary_cp850_8x16_psfu_start;
    terminal.font_bitmap = (uint8_t *)terminal.font_header + terminal.font_header->offset;
}

void terminal_set_cursor(uint32_t x, uint32_t y)
{
    if (x > terminal.width) {
        x = terminal.width;
    }
    if (y > terminal.height) {
        y = terminal.height;
    }
    terminal.cursor.x = x;
    terminal.cursor.y = y;
}

static void terminal_print_char(char c)
{
    if (terminal.cursor.x > terminal.width) {
        terminal.cursor.x = 0;
        terminal.cursor.y++;
    }

    if (terminal.cursor.y > terminal.height) {
        terminal.cursor.y = terminal.height - 1;
    }

    if (c == '\n') {
        terminal.cursor.x = 0;
        terminal.cursor.y++;
        return;
    }

    uint8_t *glyph = terminal.font_bitmap + (c * terminal.font_header->height);
    display_draw_bitmap(terminal.cursor.x * terminal.font_header->width,
                        terminal.cursor.y * terminal.font_header->height,
                        terminal.font_header->width, terminal.font_header->height,
                        glyph);
    terminal.cursor.x++;
}

static void terminal_print_string(string_t str)
{
    for (uint32_t i = 0; i < str.size; i++) {
        terminal_print_char(string_at(str, i));
    }
}

void terminal_print(string_t format, ...)
{
    va_list ap;
    char msg_data[MSG_MAX_LENGTH];
    string_t msg = string_init(msg_data, 0, MSG_MAX_LENGTH);

    va_start(ap, format);
    string_format(&msg, format, ap);
    va_end(ap);

    terminal_print_string(msg);
}

void terminal_print_at(uint32_t x, uint32_t y, string_t format, ...)
{
    va_list ap;
    char msg_data[MSG_MAX_LENGTH];
    string_t msg = string_init(msg_data, 0, MSG_MAX_LENGTH);
    va_start(ap, format);
    string_format(&msg, format, ap);
    va_end(ap);
    terminal_set_cursor(x, y);
    terminal_print_string(msg);
}
