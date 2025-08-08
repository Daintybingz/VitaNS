#ifndef SWITCH_NATIVE_WINDOW_H
#define SWITCH_NATIVE_WINDOW_H

#include <stddef.h>
#include <stdint.h>

enum switch_pixel_format {
    SWITCH_FORMAT_RGBA8888 = 1,
    SWITCH_FORMAT_BGRA8888 = 2,
    SWITCH_FORMAT_RGB565   = 3,
};

typedef struct switch_native_window {
    void *userdata;
    int32_t width;
    int32_t height;
    int32_t stride_bytes;   /* bytes per row for framebuffer, if known */
    int32_t format;         /* enum switch_pixel_format */

    /* Acquire a pointer to the current framebuffer. Optional. */
    void *(*acquire_framebuffer)(void *userdata, int32_t *out_stride_bytes);

    /* Present a source image to the display. If src is NULL, implementer may clear. */
    void (*present)(void *userdata,
                    const void *src_pixels,
                    int32_t src_stride_bytes,
                    int32_t width,
                    int32_t height,
                    int32_t format);
} switch_native_window;

#endif /* SWITCH_NATIVE_WINDOW_H */


