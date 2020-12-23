#include <terminal.h>
#include <stdarg.h> 
#include <display.h>
#include <string.h>

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

void terminal_print(char *format, ...)
{
    va_list ap;
    uint32_t u;
    int32_t i;
    char *s;
    int c;
    char msg[MSG_MAX_LENGTH];
    uint32_t msg_size = 0;
    uint32_t format_size = strlen(format);
    
    va_start(ap, format);
    for (uint32_t format_index = 0; format_index < format_size; format_index++) {
        char ch = format[format_index];
        if (ch == '%') {
            
            if (format[format_index+1] == 'u') {
                u = va_arg(ap, uint32_t);
                msg_size += uint_to_str(&msg[msg_size], u, MSG_MAX_LENGTH - msg_size);
            }
            
            if (format[format_index+1] == 'i' ||
                format[format_index+1] == 'd') {
                i = va_arg(ap, int32_t);
                msg_size += int_to_str(&msg[msg_size], i, MSG_MAX_LENGTH - msg_size);
            }
            
            if (format[format_index+1] == 's') {
                s = va_arg(ap, char *);
                uint32_t copy_size = strlen(s);
                if (copy_size > MSG_MAX_LENGTH - msg_size) {
                    copy_size = MSG_MAX_LENGTH - msg_size;
                }
                strncpy(&msg[msg_size], s, copy_size);
                msg_size += copy_size;
            }
            
            if (format[format_index+1] == 'c') {
                c = va_arg(ap, int);
                if (MSG_MAX_LENGTH - msg_size > 0) {
                    msg[msg_size++] = (char)c;
                }
            }
            
            format_index++;
            
        } else {
            msg[msg_size++] = ch;
        }
    }
    va_end(ap);
    msg[msg_size] = 0;

    terminal_print_string(msg);
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
