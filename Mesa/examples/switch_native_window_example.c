#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stddef.h>
#include <stdint.h>
#include "switch_native_window.h"

static void *acquire_fb(void *userdata, int32_t *out_stride_bytes) {
    (void)userdata;
    if (out_stride_bytes) *out_stride_bytes = 0;
    return NULL;
}

static void present(void *userdata,
                    const void *src_pixels,
                    int32_t src_stride_bytes,
                    int32_t width,
                    int32_t height,
                    int32_t format) {
    (void)userdata; (void)src_pixels; (void)src_stride_bytes; (void)width; (void)height; (void)format;
}

int demo_run(void) {
    switch_native_window win = {
        .userdata = NULL,
        .width = 1280,
        .height = 720,
        .stride_bytes = 1280 * 4,
        .format = SWITCH_FORMAT_RGBA8888,
        .acquire_framebuffer = acquire_fb,
        .present = present,
    };

    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(dpy, NULL, NULL);

    EGLint cfg_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    EGLConfig cfg; EGLint num;
    eglChooseConfig(dpy, cfg_attribs, &cfg, 1, &num);

    EGLSurface surf = eglCreateWindowSurface(dpy, cfg, (EGLNativeWindowType)&win, NULL);

    EGLint ctx_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctx_attribs);

    eglMakeCurrent(dpy, surf, surf, ctx);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(dpy, surf);

    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(dpy, ctx);
    eglDestroySurface(dpy, surf);
    eglTerminate(dpy);
    return 0;
}


