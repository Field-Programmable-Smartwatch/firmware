#include <libraries/error.h>
#include <libraries/queue.h>
#include <kernel/task/task_manager.h>
#include <kernel/tasks/input_task.h>
#include <mcu/stm32wb55xx/gpio.h>
#include <mcu/stm32wb55xx/i2c.h>
#include <drivers/input/ft6336g.h>
#include <common/input_event.h>

static bool g_input_task_running = false;
static task_handle_t g_input_task_handle;
static uint32_t g_input_task_stack[500] __attribute__((aligned(4)));
static input_event_t g_input_queue_buffer[10] = {0};
static queue_handle_t g_input_queue;
static gpio_handle_t g_int_pin_handle;
static ft6336g_handle_t g_ft6336g_handle;

static error_t input_task_init_touchscreen()
{
    error_t error;
    i2c_handle_t i2c_handle;

    // Initialize touchscreen
    error = i2c_device_init((i2c_device_configuration_t)
                            {.i2c = I2C1,
                             .address_mode = I2C_ADDRESS_MODE_7BIT,
                             .address = 0x38},
                            &i2c_handle);
    if (error) {
        return error;
    }

    error = gpio_init((gpio_configuration_t)
                      {.port = GPIOA,
                       .pin = 0,
                       .mode = GPIO_MODE_INPUT,
                       .output_type = GPIO_OUTPUT_TYPE_PUSH_PULL,
                       .output_speed = GPIO_OUTPUT_SPEED_HIGH,
                       .pull_resistor = GPIO_PULL_RESISTOR_UP},
                      &g_int_pin_handle);
    if (error) {
        return error;
    }

    error = ft6336g_init((ft6336g_configuration_t)
                         {.i2c_handle = i2c_handle,
                          .threshold = 40,
                          .interrupt_mode = FT6336G_INTERRUPT_MODE_POLLING},
                         &g_ft6336g_handle);
    if (error) {
        return error;
    }

    return SUCCESS;
}

error_t input_task_init()
{
    error_t error;

    error = queue_init(10, sizeof(input_event_t), g_input_queue_buffer, &g_input_queue);
    if (error) {
        return error;
    }

    error = input_task_init_touchscreen();
    if (error) {
        return error;
    }

    error = task_manager_init_task(input_task_entry,
                                   1,
                                   500,
                                   g_input_task_stack,
                                   &g_input_task_handle);
    if (error) {
        return error;
    }

    g_input_task_running = true;
    return SUCCESS;
}

error_t input_task_get_event(input_event_t *event)
{
    if (!event) {
        return ERROR_INVALID;
    }

    if (queue_length(g_input_queue) == 0) {
        event->valid = 0;
        return SUCCESS;
    }

    return queue_pop(g_input_queue, (void *)event);
}

void input_task_entry()
{
    error_t error;

    bool display_touched = false;
    uint32_t no_touch_threshold = 0;

    while (g_input_task_running) {
        uint8_t touch_not_detected = 1;
        input_event_t event = {0};
        uint16_t x, y;
        ft6336g_event_flag_t event_flag;

        gpio_read(g_int_pin_handle, &touch_not_detected);
        if (touch_not_detected) {
            if (display_touched) {
                if (no_touch_threshold > 10000) {
                    event.valid = true;
                    event.type = 1;
                    no_touch_threshold = 0;
                    display_touched = false;
                    queue_push(g_input_queue, (void *)&event);
                }
                no_touch_threshold++;
            }
            continue;
        }

        error = ft6336g_read(g_ft6336g_handle, &x, &y, &event_flag);
        if (error) {
            continue;
        }

        event.valid = true;
        event.type = event_flag;
        event.x = x;
        event.y = y;

        if (event.type != 1) {
            display_touched = true;
        } else {
            display_touched = false;
        }

        no_touch_threshold = 0;

        queue_push(g_input_queue, (void *)&event);
    }
}
