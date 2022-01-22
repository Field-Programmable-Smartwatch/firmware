#include <mcu/stm32wb55xx/gpio.h>
#include <mcu/stm32wb55xx/spi.h>
#include <mcu/stm32wb55xx/dma.h>
#include <mcu/system_timer.h>
#include <libraries/error.h>
#include <libraries/queue.h>
#include <common/render_request.h>
#include <common/rectangle.h>
#include <drivers/display/st7789.h>
#include <kernel/task/task_manager.h>
#include <kernel/tasks/display_task.h>

extern uint16_t _framebuffer_start[];
extern uint16_t _framebuffer_end[];
static st7789_handle_t g_display_handle;
static uint32_t g_display_width = 240;
static uint32_t g_display_height = 240;
static bool g_display_task_running = false;
static task_handle_t g_display_task_handle;
static uint32_t g_display_task_stack[200] __attribute__((aligned(4)));
static render_request_t g_render_queue_buffer[10] = {0};
static queue_handle_t g_render_queue;

void dma_transfer_complete_handler(dma_handle_t handle)
{
    dma_stop(handle);
    DMA1->IFCR |= 0xfffffff;
}

static error_t display_task_init_display()
{
    error_t error;
    spi_device_handle_t display_spi_handle;
    gpio_handle_t dc_pin_handle;
    dma_handle_t spi_dma_handle;

    // Initialize display
    error = dma_init((dma_configuration_t)
                     {.mode = DMA_MODE_MEMORY_TO_PERIPHERAL,
                      .request = DMA_REQUEST_SPI1_TX,
                      .increment_source = true,
                      .transfer_complete_handler = &dma_transfer_complete_handler},
                     &spi_dma_handle);

    error = spi_device_init((spi_device_configuration_t)
                            {.spi = SPI1,
                             .cs_port = GPIOB,
                             .cs_pin = 1,
                             .clock_mode = SPI_CLOCK_MODE_0,
                             .mode = SPI_MODE_MASTER,
                             .baud_rate_prescaler = SPI_BAUD_RATE_PRESCALER_2,
                             .significant_bit = SPI_SIGNIFICANT_BIT_MSB,
                             .com_mode = SPI_COM_MODE_FULL_DUPLEX,
                             .data_size = SPI_DATA_SIZE_8BIT,
                             .active_low = true,
                             .dma_handle = spi_dma_handle},
                            &display_spi_handle);
    if (error) {
        return error;
    }

    error = gpio_init((gpio_configuration_t)
                      {.port = GPIOB,
                       .pin = 0,
                       .mode = GPIO_MODE_OUTPUT,
                       .output_type = GPIO_OUTPUT_TYPE_PUSH_PULL,
                       .output_speed = GPIO_OUTPUT_SPEED_HIGH,
                       .pull_resistor = GPIO_PULL_RESISTOR_NONE},
                      &dc_pin_handle);
    if (error) {
        return error;
    }

    error = st7789_init((st7789_configuration_t)
                        {.spi_handle = display_spi_handle,
                         .dc_pin_handle = dc_pin_handle,
                         .framebuffer_size = g_display_width * g_display_height,
                         .color_format = ST7789_COLOR_FORMAT_16_BIT,
                         .width = g_display_width,
                         .height = g_display_height},
                        &g_display_handle);
    if (error) {
        return error;
    }

    st7789_clear(g_display_handle);

    return SUCCESS;
}

static error_t display_task_validate_rectangle(const rectangle_t *rect)
{
    // TODO: Implement
    return SUCCESS;
}

static error_t display_task_get_framebuffer_data(uint32_t x, uint32_t y, uint16_t **frame_data)
{
    if (!frame_data) {
        return ERROR_INVALID;
    }

    *frame_data = (uint16_t *)_framebuffer_start + (g_display_width * y) + x;
    return SUCCESS;
}

static error_t display_task_render_dirty_rect(rectangle_t dirty_rect)
{
    error_t error;
    uint16_t *framebuffer_data;

    error = display_task_validate_rectangle(&dirty_rect);
    if (error) {
        return error;
    }

    error = display_task_get_framebuffer_data(dirty_rect.x, dirty_rect.y, &framebuffer_data);
    if (error) {
        return error;
    }

    error = st7789_draw_rect(g_display_handle, dirty_rect.x, dirty_rect.y,
                             dirty_rect.width, dirty_rect.height,
                             framebuffer_data);
    if (error) {
        return error;
    }

    return SUCCESS;
}

error_t display_task_init()
{
    error_t error;

    error = queue_init(10, sizeof(render_request_t), g_render_queue_buffer, &g_render_queue);
    if (error) {
        return error;
    }

    error = display_task_init_display();
    if (error) {
        return error;
    }

    error = task_manager_init_task(display_task_entry,
                                   1,
                                   200,
                                   g_display_task_stack,
                                   &g_display_task_handle);
    if (error) {
        return error;
    }

    g_display_task_running = true;
    return SUCCESS;
}

void display_task_entry()
{
    while (g_display_task_running) {
        render_request_t request = {0};

        if (queue_length(g_render_queue) == 0) {
            continue;
        }

        queue_pop(g_render_queue, &request);

        display_task_render_dirty_rect(request.rect);
        *request.render_complete = true;
    }
}

error_t display_task_queue_request(render_request_t *request)
{
    return queue_push(g_render_queue, request);
}
