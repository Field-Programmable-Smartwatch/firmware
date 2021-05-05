#include <accelerometer.h>
#include <i2c.h>
#include <log.h>
#include <error.h>

#define MMA8451_DEVICE_ADDRESS 0x1d
#define MMA8451_REG_OUT_X_MSB (0x01)

error_t accelerometer_open(uint32_t *handle)
{
    error_t error;
    i2c_configuration_t config;

    config.address_mode = I2C_ADDRESS_MODE_7BIT;
    config.address = MMA8451_DEVICE_ADDRESS;
    error = i2c_open(handle, config);
    if (error) {
        log_error(error, "Failed to initialize accelerometer");
        return error;
    }

    // Set active mode and low noise mode
    uint8_t data[2];
    data[0] = 0x2a;
    data[1] = 0x01 | 0x4;
    i2c_write(*handle, data, 2);
    i2c_stop(*handle);

    return SUCCESS;
}

error_t accelerometer_close(uint32_t handle)
{
    return i2c_close(handle);
}

error_t accelerometer_read(uint32_t handle, uint32_t *x, uint32_t *y, uint32_t *z)
{
    uint32_t value = 0;
    *x = 0;
    *y = 0;
    *z = 0;

    i2c_write_byte(handle, 0x1);
    i2c_read_byte(handle, (uint8_t *)&value);
    i2c_stop(handle);
    *x |= value << 8;
    value = 0;

    i2c_write_byte(handle, 0x2);
    i2c_read_byte(handle, (uint8_t *)&value);
    i2c_stop(handle);
    *x |= value >> 2;
    value = 0;

    i2c_write_byte(handle, 0x3);
    i2c_read_byte(handle, (uint8_t *)&value);
    i2c_stop(handle);
    *y |= value << 8;
    value = 0;

    i2c_write_byte(handle, 0x4);
    i2c_read_byte(handle, (uint8_t *)&value);
    i2c_stop(handle);
    *y |= value >> 2;
    value = 0;

    i2c_write_byte(handle, 0x5);
    i2c_read_byte(handle, (uint8_t *)&value);
    i2c_stop(handle);
    *z |= value << 8;
    value = 0;

    i2c_write_byte(handle, 0x6);
    i2c_read_byte(handle, (uint8_t *)&value);
    i2c_stop(handle);
    *z |= value >> 2;
    value = 0;

    return SUCCESS;
}

error_t acceleromter_ioctl(uint32_t handle)
{
    return SUCCESS;
}
