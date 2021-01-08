#include <terminal.h>
#include <stdarg.h> 
#include <display.h>
#include <string.h>
#include <debug.h>

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

void terminal_print_char(char c)
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

void terminal_print_string(char *str)
{
    while (*str) {
        terminal_print_char(*str++);
    }
}

void _format_string(char *format, va_list ap, char *str, uint32_t *str_len)
{
    uint32_t u;
    int32_t i;
    char *s;
    int c;
    uint32_t format_size = strlen(format);

    *str_len = 0;
    for (uint32_t format_index = 0; format_index < format_size; format_index++) {
        char ch = format[format_index];
        if (ch == '%') {
            
            if (format[format_index+1] == 'u') {
                u = va_arg(ap, uint32_t);
                *str_len += uint_to_str(&str[*str_len], u, MSG_MAX_LENGTH - *str_len);
            }
            
            if (format[format_index+1] == 'i' ||
                format[format_index+1] == 'd') {
                i = va_arg(ap, int32_t);
                *str_len += int_to_str(&str[*str_len], i, MSG_MAX_LENGTH - *str_len);
            }
            
            if (format[format_index+1] == 's') {
                s = va_arg(ap, char *);
                uint32_t copy_size = strlen(s);
                if (copy_size > MSG_MAX_LENGTH - *str_len) {
                    copy_size = MSG_MAX_LENGTH - *str_len;
                }
                strncpy(&str[*str_len], s, copy_size);
                *str_len += copy_size;
            }
            
            if (format[format_index+1] == 'c') {
                c = va_arg(ap, int);
                if (MSG_MAX_LENGTH - *str_len > 0) {
                    str[(*str_len)++] = (char)c;
                }
            }
            
            format_index++;
            
        } else {
            str[(*str_len)++] = ch;
        }
    }
}

void terminal_print(char *format, ...)
{
    va_list ap;
    char msg[MSG_MAX_LENGTH];

    va_start(ap, format);
    string_format(format, ap, msg, MSG_MAX_LENGTH);
    va_end(ap);

    terminal_print_string(msg);
}

void terminal_print_at(uint32_t x, uint32_t y, char *format, ...)
{
    va_list ap;
    char msg[MSG_MAX_LENGTH];
    va_start(ap, format);
    string_format(format, ap, msg, MSG_MAX_LENGTH);
    va_end(ap);

    terminal_set_cursor(x, y);
    terminal_print_string(msg);
}
