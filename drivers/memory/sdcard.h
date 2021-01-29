#ifndef SDCARD_H
#define SDCARD_H

#include <stdint.h>

void sdcard_read_block(uint32_t addr, void *buffer);
void sdcard_write_block(uint32_t addr, void *buffer);
void sdcard_init();

#endif
