#include <stdint.h>
#include <libraries/error.h>
#include <libraries/string.h>
#include <libraries/graphics.h>
#include <libraries/system.h>
#include <common/rectangle.h>
#include <common/render_request.h>

extern uint16_t _framebuffer_start[];
extern uint16_t _framebuffer_end[];
static uint32_t g_display_width;
// static uint32_t g_display_height;

static error_t graphics_validate_rect(rectangle_t rect)
{
    return SUCCESS;
}

static error_t graphics_get_framebuffer_address(rectangle_t rect, color16_t **framebuffer)
{
    if (!framebuffer) {
        return ERROR_INVALID;
    }

    *framebuffer = (color16_t *)_framebuffer_start + (g_display_width * rect.y) + rect.x;
    return SUCCESS;
}

error_t graphics_init(uint32_t display_width, uint32_t display_height)
{
    // TODO: Validate display width and height
    g_display_width = display_width;
    g_display_width = display_height;
    return SUCCESS;
}

error_t graphics_draw_rectangle(rectangle_t rect, color16_t color)
{
    error_t error;
    color16_t *framebuffer;
    render_request_t request = {0};
    bool render_complete = false;

    error = graphics_validate_rect(rect);
    if (error) {
        return error;
    }

    error = graphics_get_framebuffer_address(rect, &framebuffer);
    if (error) {
        return error;
    }

    for (uint32_t i = 0; i < rect.height; i++) {
        memset16(framebuffer, color, rect.width);
        framebuffer += g_display_width;
    }

    request.rect = rect;
    request.render_complete = &render_complete;

    system_call(SYSTEM_CALL_RENDER, (void *)&request);
    while (!render_complete);

    return SUCCESS;
}
