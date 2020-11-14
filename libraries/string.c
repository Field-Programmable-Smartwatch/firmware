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
