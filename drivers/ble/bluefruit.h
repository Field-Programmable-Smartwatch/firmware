#ifndef BLUEFRUIT_H
#define BLUEFRUIT_H

#include <stdint.h>
#include <error.h>

error_t bluefruit_read(void *buffer, uint32_t size);
error_t bluefruit_write(void *buffer, uint32_t size);
error_t bluefruit_init();

#endif
