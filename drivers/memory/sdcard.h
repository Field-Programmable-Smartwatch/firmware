#ifndef SDCARD_H
#define SDCARD_H

#include <stdint.h>
#include <error.h>

error_t sdcard_read_block(uint32_t addr, void *buffer);
error_t sdcard_write_block(uint32_t addr, void *buffer);
error_t sdcard_init();

#endif
