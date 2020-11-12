#ifndef STRING_H
#define STRING_H

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



#endif
