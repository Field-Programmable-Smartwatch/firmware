#include <stdint.h>

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

uint32_t uint_to_str(char *dest, uint32_t u, const uint32_t size)
{
    if (size == 0) {
        return 0;
    }
    
    char str[11];
    str[10] = 0;
    uint32_t str_index = 10;
    uint32_t bytes_copied = 0;
    
    do {
        uint8_t digit = u % 10;
        str[--str_index] = digit | 48; // convert raw digit to ascii representation
        u -= digit;
        u /= 10;
    } while (u);

    while (bytes_copied < size && str[str_index]) {
        dest[bytes_copied++] = str[str_index++];
    }
    
    return bytes_copied;
}

uint32_t int_to_str(char *dest, const int32_t i, const uint32_t size)
{
    if (i > 0) {
        return uint_to_str(dest, i, size);
    }

    uint32_t u = ~(i) + 1;
    
    char str[11];
    str[10] = 0;
    uint32_t str_index = 10;
    uint32_t bytes_copied = 0;
    
    do {
        uint8_t digit = u % 10;
        str[str_index--] = digit | 48; // convert raw digit to ascii representation
        u -= digit;
        u /= 10;
    } while (u);

    str[str_index] = '-';
    while (bytes_copied < size && str[str_index]) {
        dest[bytes_copied++] = str[str_index++];
    }

    return bytes_copied;
}
