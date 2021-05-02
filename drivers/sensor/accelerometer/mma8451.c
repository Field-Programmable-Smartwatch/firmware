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

    return SUCCESS;
}

error_t accelerometer_close(uint32_t handle)
{
    return i2c_close(handle);
}

error_t accelerometer_read(uint32_t handle, uint32_t *x, uint32_t *y, uint32_t *z)
{
    log_debug("a");
    uint8_t out_x_msb_register_addr = MMA8451_REG_OUT_X_MSB;
    i2c_write(handle, &out_x_msb_register_addr, 1);
    uint32_t value;
    log_debug("b");

    i2c_read(handle, (uint8_t *)&value, 1);
    i2c_write(handle, &out_x_msb_register_addr, 1);
    while(1);
    /* *x = value << 8; */
    /* i2c_read(handle, (uint8_t *)&value, 8); */
    /* *x |= value >> 2; */
    /* log_debug("c"); */

    /* i2c_read(handle, &value, 8); */
    /* *y = value << 8; */
    /* i2c_read(handle, &value, 8); */
    /* *y |= value >> 2; */
    /* log_debug("d"); */

    /* i2c_read(handle, &value, 8); */
    /* *z = value << 8; */
    /* i2c_read(handle, &value, 8); */
    /* *z |= value >> 2; */
    /* log_debug("e"); */

    return SUCCESS;
}

error_t acceleromter_ioctl(uint32_t handle)
{
    return SUCCESS;
}
