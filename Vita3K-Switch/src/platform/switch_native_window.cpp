#include "switch_native_window.h"
#include <switch.h>
#include <cstring>

// Basic Switch native window implementation
// This provides the required callbacks for Mesa's EGL driver

struct SwitchWindowContext {
    NWindow *nwin;
    Framebuffer fb;
    void *framebuffer_ptr;
    int32_t stride_bytes;
};

static void *acquire_framebuffer(void *userdata, int32_t *out_stride_bytes) {
    SwitchWindowContext *ctx = (SwitchWindowContext *)userdata;
    if (!ctx) return nullptr;
    
    framebufferBegin(&ctx->fb, NULL);
    ctx->framebuffer_ptr = framebufferGetPixels(&ctx->fb);
    if (out_stride_bytes) {
        *out_stride_bytes = framebufferGetStride(&ctx->fb);
    }
    return ctx->framebuffer_ptr;
}

static void present(void *userdata,
                   const void *src_pixels,
                   int32_t src_stride_bytes,
                   int32_t width,
                   int32_t height,
                   int32_t format) {
    SwitchWindowContext *ctx = (SwitchWindowContext *)userdata;
    if (!ctx) return;
    
    // If no source pixels provided, clear to a dark blue color
    if (!src_pixels) {
        // Clear to dark blue (RGBA: 0x10, 0x20, 0x40, 0xFF)
        uint32_t *dst = (uint32_t *)ctx->framebuffer_ptr;
        uint32_t dark_blue = 0xFF402010; // RGBA8888 format
        int32_t dst_stride = framebufferGetStride(&ctx->fb) / 4; // Convert to pixel count
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                dst[y * dst_stride + x] = dark_blue;
            }
        }
    } else {
        // Copy source pixels to framebuffer
        uint8_t *dst = (uint8_t *)ctx->framebuffer_ptr;
        const uint8_t *src = (const uint8_t *)src_pixels;
        int32_t dst_stride = framebufferGetStride(&ctx->fb);
        
        // Copy row by row
        int32_t copy_width = (src_stride_bytes < dst_stride) ? src_stride_bytes : dst_stride;
        for (int y = 0; y < height; ++y) {
            memcpy(dst + (size_t)y * (size_t)dst_stride,
                   src + (size_t)y * (size_t)src_stride_bytes,
                   (size_t)copy_width);
        }
    }
    
    framebufferEnd(&ctx->fb);
}

extern "C" int switch_native_window_create(switch_native_window *out, int width, int height) {
    if (!out) return -1;
    
    // Initialize video service
    viInitialize(ViServiceType_Default);
    
    // Create window and framebuffer
    SwitchWindowContext *ctx = new SwitchWindowContext();
    nwindowCreate(&ctx->nwin);
    framebufferCreate(&ctx->fb, ctx->nwin, width, height, PIXEL_FORMAT_RGBA_8888, 2);
    framebufferMakeLinear(&ctx->fb);
    
    // Setup the native window structure
    out->userdata = ctx;
    out->width = width;
    out->height = height;
    out->stride_bytes = width * 4; // RGBA8888 = 4 bytes per pixel
    out->format = SWITCH_FORMAT_RGBA8888;
    out->acquire_framebuffer = acquire_framebuffer;
    out->present = present;
    
    return 0;
}

extern "C" void switch_native_window_destroy(switch_native_window *window) {
    if (!window || !window->userdata) return;
    
    SwitchWindowContext *ctx = (SwitchWindowContext *)window->userdata;
    framebufferClose(&ctx->fb);
    nwindowClose(ctx->nwin);
    delete ctx;
    
    window->userdata = nullptr;
}
