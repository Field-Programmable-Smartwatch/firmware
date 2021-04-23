#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>
#include <error.h>

typedef uint32_t i2c_handle_t;

typedef enum i2c_address_mode {
    I2C_ADDRESS_MODE_7BIT,
    I2C_ADDRESS_MODE_10BIT
} i2c_address_mode_t;

typedef struct i2c_device {
    bool is_open;
    i2c_address_mode_t address_mode;
    uint16_t address;
} i2c_device_t;

error_t i2c_read(i2c_handle_t handle, uint8_t *data, uint32_t size);
error_t i2c_write(i2c_handle_t handle, uint8_t *data, uint32_t size);
error_t i2c_open( i2c_handle_t *handle, i2c_device_t device);
error_t i2c_init();
error_t i2c_deinit();

#endif
