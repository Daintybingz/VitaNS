// Reference-only libnx example (not built here)
#include <switch.h>
#include <stdint.h>
#include "switch_native_window.h"

typedef struct {
    NWindow *nwin;
    Framebuffer fb;
} AppFb;

static void *acquire_fb(void *userdata, int32_t *out_stride_bytes) {
    AppFb *app = (AppFb *)userdata;
    framebufferBegin(&app->fb, NULL);
    if (out_stride_bytes) *out_stride_bytes = framebufferGetStride(&app->fb);
    return framebufferGetPixels(&app->fb);
}

static void present(void *userdata,
                    const void *src_pixels,
                    int32_t src_stride_bytes,
                    int32_t width,
                    int32_t height,
                    int32_t format) {
    (void)format; (void)width; (void)height;
    AppFb *app = (AppFb *)userdata;
    u8 *dst = framebufferGetPixels(&app->fb);
    const u8 *src = (const u8 *)src_pixels;
    int32_t dst_stride = framebufferGetStride(&app->fb);
    if (src && dst) {
        int32_t copy = src_stride_bytes < dst_stride ? src_stride_bytes : dst_stride;
        for (int y = 0; y < height; ++y) {
            memcpy(dst + (size_t)y * (size_t)dst_stride,
                   src + (size_t)y * (size_t)src_stride_bytes,
                   (size_t)copy);
        }
    }
    framebufferEnd(&app->fb);
}

int setup_native_window(switch_native_window *out, AppFb *ctx, int width, int height) {
    if (!out || !ctx) return -1;
    // Init video and create window
    viInitialize(ViServiceType_Default);
    nwindowCreate(&ctx->nwin);
    framebufferCreate(&ctx->fb, ctx->nwin, width, height, PIXEL_FORMAT_RGBA_8888, 2);
    framebufferMakeLinear(&ctx->fb);

    out->userdata = ctx;
    out->width = width;
    out->height = height;
    out->stride_bytes = width * 4;
    out->format = SWITCH_FORMAT_RGBA8888;
    out->acquire_framebuffer = NULL; // using present() path below
    out->present = present;
    return 0;
}


