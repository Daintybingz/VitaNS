// Early Switch EGL driver skeleton
// Goal: keep build green while preparing a path to real context/surface creation.

#include <stdlib.h>
#include "egldefines.h"
#include "egldisplay.h"
#include "egldriver.h"
#include "eglcontext.h"
#include "eglsurface.h"
#include "switch_native_window.h"
#include <string.h>

// Minimal private structs (placeholders)
struct switch_display { int reserved; };
struct switch_context { _EGLContext base; };
struct switch_surface {
    _EGLSurface base;
    EGLint swap_interval;
    switch_native_window *native;
    void *pixel_buffer;
    size_t pixel_buffer_size;
};

static inline struct switch_surface *switch_surface(_EGLSurface *surf) {
    return (struct switch_surface *)surf;
}

static EGLBoolean switch_initialize(_EGLDisplay *disp) {
    (void)disp;
    // TODO: choose API mask and default config list
    return EGL_TRUE;
}

static EGLBoolean switch_terminate(_EGLDisplay *disp) {
    (void)disp;
    return EGL_TRUE;
}

static _EGLContext *switch_create_context(_EGLDisplay *disp, _EGLConfig *config,
                                          _EGLContext *share_list,
                                          const EGLint *attrib_list) {
    struct switch_context *ctx = calloc(1, sizeof(*ctx));
    if (!ctx)
        return NULL;
    if (!_eglInitContext(&ctx->base, disp, config, share_list, attrib_list)) {
        free(ctx);
        return NULL;
    }
    return &ctx->base;
}

static EGLBoolean switch_destroy_context(_EGLDisplay *disp, _EGLContext *ctx) {
    (void)disp;
    if (_eglPutContext(ctx)) {
        struct switch_context *sctx = (struct switch_context *)ctx;
        free(sctx);
    }
    return EGL_TRUE;
}

static EGLBoolean switch_make_current(_EGLDisplay *disp, _EGLSurface *draw,
                                      _EGLSurface *read, _EGLContext *ctx) {
    (void)disp;
    _EGLContext *old_ctx = NULL;
    _EGLSurface *old_dsurf = NULL;
    _EGLSurface *old_rsurf = NULL;
    if (!_eglBindContext(ctx, draw, read, &old_ctx, &old_dsurf, &old_rsurf))
        return EGL_FALSE;
    _eglPutSurface(old_dsurf);
    _eglPutSurface(old_rsurf);
    _eglPutContext(old_ctx);
    return EGL_TRUE;
}

static _EGLSurface *switch_create_pbuffer_surface(_EGLDisplay *disp, _EGLConfig *config,
                                                  const EGLint *attrib_list) {
    struct switch_surface *surf = calloc(1, sizeof(*surf));
    if (!surf)
        return NULL;
    if (!_eglInitSurface(&surf->base, disp, EGL_PBUFFER_BIT, config, attrib_list, NULL)) {
        free(surf);
        return NULL;
    }
    surf->swap_interval = 1;
    /* Allocate a simple RGBA8888 buffer for stub present path */
    surf->pixel_buffer_size = (size_t)surf->base.Width * (size_t)surf->base.Height * 4u;
    surf->pixel_buffer = (surf->pixel_buffer_size > 0) ? malloc(surf->pixel_buffer_size) : NULL;
    return &surf->base;
}

static _EGLSurface *switch_create_window_surface(_EGLDisplay *disp, _EGLConfig *config,
                                                 void *native_window,
                                                 const EGLint *attrib_list) {
    struct switch_surface *surf = calloc(1, sizeof(*surf));
    if (!surf)
        return NULL;
    if (!_eglInitSurface(&surf->base, disp, EGL_WINDOW_BIT, config, attrib_list, native_window)) {
        free(surf);
        return NULL;
    }
    surf->swap_interval = 1;
    surf->native = (switch_native_window *)native_window;
    surf->pixel_buffer_size = (size_t)surf->base.Width * (size_t)surf->base.Height * 4u;
    surf->pixel_buffer = (surf->pixel_buffer_size > 0) ? malloc(surf->pixel_buffer_size) : NULL;
    return &surf->base;
}

static EGLBoolean switch_destroy_surface(_EGLDisplay *disp, _EGLSurface *surface) {
    (void)disp;
    if (_eglPutSurface(surface)) {
        struct switch_surface *ssurf = (struct switch_surface *)surface;
        if (ssurf->pixel_buffer) {
            free(ssurf->pixel_buffer);
            ssurf->pixel_buffer = NULL;
            ssurf->pixel_buffer_size = 0;
        }
        free(ssurf);
    }
    return EGL_TRUE;
}

static EGLBoolean switch_query_surface(_EGLDisplay *disp, _EGLSurface *surface,
                                       EGLint attribute, EGLint *value) {
    (void)disp;
    if (!surface || !value)
        return EGL_FALSE;
    struct switch_surface *ssurf = switch_surface(surface);
    switch (attribute) {
    case EGL_WIDTH:
        *value = ssurf->base.Width;
        return EGL_TRUE;
    case EGL_HEIGHT:
        *value = ssurf->base.Height;
        return EGL_TRUE;
    case EGL_SWAP_BEHAVIOR:
        *value = EGL_BUFFER_DESTROYED;
        return EGL_TRUE;
    default:
        return EGL_FALSE;
    }
}

static EGLBoolean switch_swap_interval(_EGLDisplay *disp, _EGLSurface *surf,
                                       EGLint interval) {
    (void)disp;
    if (!surf)
        return EGL_FALSE;
    struct switch_surface *ssurf = switch_surface(surf);
    if (interval < 0)
        interval = 0;
    ssurf->swap_interval = interval;
    return EGL_TRUE;
}

static EGLBoolean switch_swap_buffers(_EGLDisplay *disp, _EGLSurface *draw) {
    (void)disp;
    if (!draw)
        return EGL_FALSE;
    struct switch_surface *ssurf = switch_surface(draw);
    switch_native_window *nw = ssurf->native;
    if (nw) {
        /* For now, fill a solid color into our stub pixel buffer */
        const int32_t stride = ssurf->base.Width * 4;
        if (ssurf->pixel_buffer && ssurf->pixel_buffer_size >= (size_t)(stride * ssurf->base.Height)) {
            uint8_t *p = (uint8_t *)ssurf->pixel_buffer;
            /* Simple color pattern based on swap interval and size */
            const uint8_t r = 0x10;
            const uint8_t g = 0x20;
            const uint8_t b = 0x40;
            const uint8_t a = 0xFF;
            for (int y = 0; y < ssurf->base.Height; ++y) {
                uint8_t *row = p + (size_t)y * (size_t)stride;
                for (int x = 0; x < ssurf->base.Width; ++x) {
                    size_t o = (size_t)x * 4u;
                    row[o+0] = r;
                    row[o+1] = g;
                    row[o+2] = b;
                    row[o+3] = a;
                }
            }
        }
        if (nw->acquire_framebuffer && !nw->present) {
            int32_t fb_stride = 0;
            void *fb = nw->acquire_framebuffer(nw->userdata, &fb_stride);
            if (fb && ssurf->pixel_buffer) {
                const uint8_t *src = (const uint8_t *)ssurf->pixel_buffer;
                uint8_t *dst = (uint8_t *)fb;
                const int32_t src_stride = stride;
                const int32_t dst_stride = fb_stride > 0 ? fb_stride : stride;
                const int32_t copy_stride = src_stride < dst_stride ? src_stride : dst_stride;
                for (int y = 0; y < ssurf->base.Height; ++y) {
                    const uint8_t *s = src + (size_t)y * (size_t)src_stride;
                    uint8_t *d = dst + (size_t)y * (size_t)dst_stride;
                    memcpy(d, s, (size_t)copy_stride);
                }
            }
        } else if (nw->present) {
            nw->present(nw->userdata, ssurf->pixel_buffer, stride,
                        ssurf->base.Width, ssurf->base.Height,
                        SWITCH_FORMAT_RGBA8888);
        }
    }
    return EGL_TRUE;
}

const struct _egl_driver _eglDriver = {
    .Initialize = switch_initialize,
    .Terminate = switch_terminate,
    .CreateContext = switch_create_context,
    .DestroyContext = switch_destroy_context,
    .MakeCurrent = switch_make_current,
    .CreateWindowSurface = switch_create_window_surface,
    .CreatePbufferSurface = switch_create_pbuffer_surface,
    .DestroySurface = switch_destroy_surface,
    .QuerySurface = switch_query_surface,
    .SwapInterval = switch_swap_interval,
    .SwapBuffers = switch_swap_buffers,
};


