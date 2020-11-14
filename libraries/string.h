#ifndef STRING_H
#define STRING_H

#include <stdint.h>

void *memcpy(void *dest, void *src, uint32_t size);
void *memset(void *dest, uint8_t value, uint32_t size);
uint32_t strlen(const char *s);
char *strncpy(char *dest, const char *src, uint32_t size);

#endif
