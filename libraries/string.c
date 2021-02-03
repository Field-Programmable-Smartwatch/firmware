#include <stdint.h>
#include <stdarg.h>
#include <debug.h>

#define INT_STR_MAX_LENGTH 12

char g_char_lut[] = "0123456789abcdef";

void *memcpy(void *dest, void *src, uint32_t size)
{
    uint8_t *d = dest;
    uint8_t *s = src;
    for (uint32_t i = 0; i < size; i++) {
        d[i] = s[i];
    }

    return dest;
}

void *memset(void *dest, uint8_t value, uint32_t size)
{
    uint8_t *d = dest;

    for (uint32_t i = 0; i < size; i++) {
            d[i] = value;
    }

    return dest;
}

uint32_t strlen(const char *s)
{
    uint32_t len = 0;
    while (*s++ != '\0') {
        len++;
    }
    return len;
}

char *strncpy(char *dest, const char *src, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        dest[i] = src[i];
        if (src[i] == 0) {
            break;
        }
    }
    return dest;
}

int32_t strncmp(const char* str1, const char *str2, uint32_t size)
{
    for (uint32_t i = 0;;i++) {
        if (str1[i] < str2[i]) {
            return -1;
        }

        if (str1[i] > str2[i]) {
            return 1;
        }

        if (str1[i] == 0) {
            break;
        }
    }
    return 0;
}

uint32_t uint_to_str(char *dest, uint32_t u, const uint32_t size, uint32_t zero_padding, uint32_t base)
{
    if (size == 0) {
        return 0;
    }
    
    if (zero_padding > INT_STR_MAX_LENGTH - 1) {
        zero_padding = INT_STR_MAX_LENGTH - 1;
    }
    
    char str[INT_STR_MAX_LENGTH];
    str[INT_STR_MAX_LENGTH - 1] = 0;
    uint32_t str_index = INT_STR_MAX_LENGTH - 1;
    uint32_t bytes_copied = 0;
    
    do {
        uint8_t digit = u % base;
        str[--str_index] = g_char_lut[digit]; // convert raw digit to ascii representation
        u -= digit;
        u /= base;
        if (zero_padding) {
            zero_padding--;
        }
    } while (u || zero_padding);

    if (base == 16) {
        str[--str_index] = 'x';
        str[--str_index] = '0';
    }
    
    while (bytes_copied < size && str[str_index]) {
        dest[bytes_copied++] = str[str_index++];
    }
    
    return bytes_copied;
}

uint32_t int_to_str(char *dest, const int32_t i, const uint32_t size, uint32_t zero_padding, uint32_t base)
{
    if (i > 0) {
        return uint_to_str(dest, i, size, zero_padding, base);
    }

    if (zero_padding > 9) {
        zero_padding = 9;
    }
    
    uint32_t u = ~(i) + 1;
    
    char str[11];
    str[10] = 0;
    uint32_t str_index = 10;
    uint32_t bytes_copied = 0;
    
    do {
        uint8_t digit = u % base;
        str[str_index--] = g_char_lut[digit]; // convert raw digit to ascii representation
        u -= digit;
        u /= base;
        if (zero_padding) {
            zero_padding--;
        }
    } while (u || zero_padding);

    str[str_index] = '-';
    while (bytes_copied < size && str[str_index]) {
        dest[bytes_copied++] = str[str_index++];
    }

    return bytes_copied;
}

uint32_t str_to_uint(char *str)
{
    if (!str) {
        return 0;
    }
    uint32_t value = 0;
    for ( ; ; str += 1) {
        uint32_t digit = *str - '0';
        if (digit > 9) {
            break;
        }
        value = (value * 10) + digit;
    }    
    
    return value;
}

void string_format(char *format, va_list ap, char *dest, uint32_t dest_size)
{
    if (!format || !dest) {
        return;
    }
    uint32_t u;
    int32_t i;
    char *s;
    int c;
    uint32_t format_size = strlen(format);
    uint32_t dest_length = 0;
    uint32_t zero_padding = 0;
    for (uint32_t format_index = 0; format_index < format_size; format_index++) {
        char ch = format[format_index];
        if (ch == '%') {
            if (format[format_index+1] == '0') {
                char *format_ptr = &format[format_index+2];
                char zero_padding_str[11];
                for (uint32_t i = 0; i < 11; i++) {
                    if (format_index + 2 + i >= format_size) { // bounds check
                        format_index += 1 + i;
                        break;
                    }
                    if (format_ptr[i] < 48 || format_ptr[i] > 57) { // if not a number
                        format_index += 2+(i-1);
                        zero_padding_str[i] = 0;
                        zero_padding = str_to_uint(zero_padding_str);
                        break;
                    }
                    zero_padding_str[i] = format_ptr[i];
                }
            }

            if (format[format_index+1] == 'u') {
                u = va_arg(ap, uint32_t);
                dest_length += uint_to_str(&dest[dest_length], u, dest_size - dest_length, zero_padding, 10);
            }
            
            if (format[format_index+1] == 'i' ||
                format[format_index+1] == 'd') {
                i = va_arg(ap, int32_t);
                dest_length += int_to_str(&dest[dest_length], i, dest_size - dest_length, zero_padding, 10);
            }

            if (format[format_index+1] == 'x') {
                u = va_arg(ap, uint32_t);
                dest_length += uint_to_str(&dest[dest_length], u, dest_size - dest_length, zero_padding, 16);
            }
            
            if (format[format_index+1] == 's') {
                s = va_arg(ap, char *);
                uint32_t copy_size = strlen(s);
                if (copy_size >= dest_size - dest_length) {
                    break;
                }
                strncpy(&dest[dest_length], s, copy_size);
                dest_length += copy_size;
            }
            
            if (format[format_index+1] == 'c') {
                c = va_arg(ap, int);
                if (dest_size - dest_length <= 1) {
                    break;
                }
                dest[(dest_length)++] = (char)c;
            }
            zero_padding = 0;
            format_index++;
            
        } else {
            if (dest_size - dest_length <= 1) {
                break;
            }
            dest[(dest_length)++] = ch;
        }
    }
    dest[dest_length] = 0;
}
