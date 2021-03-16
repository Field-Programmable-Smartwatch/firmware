#ifndef BLUEFRUIT_H
#define BLUEFRUIT_H

#include <stdint.h>

void bluefruit_read(void *buffer, uint32_t size);
void bluefruit_write(void *buffer, uint32_t size);
void bluefruit_init();

#endif
