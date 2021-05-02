#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdint.h>
#include <error.h>

error_t accelerometer_open(uint32_t *handle);
error_t accelerometer_close(uint32_t handle);
error_t accelerometer_read(uint32_t handle, uint32_t *x, uint32_t *y, uint32_t *z);
error_t acceleromter_ioctl(uint32_t handle);

#endif
