#include <switch.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "platform/switch_native_window.h"
#include <cstdio>

int main(int argc, char* argv[]) {
    // Initialize console for debug output
    consoleInit(nullptr);
    printf("VitaNS: Starting Mesa EGL test...\n");
    
    // Create native window
    switch_native_window native_win;
    if (switch_native_window_create(&native_win, 1280, 720) != 0) {
        printf("Failed to create native window\n");
        return -1;
    }
    printf("Native window created: %dx%d\n", native_win.width, native_win.height);
    
    // Initialize EGL
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        printf("Failed to get EGL display\n");
        switch_native_window_destroy(&native_win);
        return -1;
    }
    printf("EGL display obtained\n");
    
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        printf("Failed to initialize EGL\n");
        switch_native_window_destroy(&native_win);
        return -1;
    }
    printf("EGL initialized: version %d.%d\n", major, minor);
    
    // Choose EGL config
    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    
    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(display, config_attribs, &config, 1, &num_configs)) {
        printf("Failed to choose EGL config\n");
        switch_native_window_destroy(&native_win);
        return -1;
    }
    printf("EGL config chosen\n");
    
    // Create EGL surface
    EGLSurface surface = eglCreateWindowSurface(display, config, 
                                               (EGLNativeWindowType)&native_win, nullptr);
    if (surface == EGL_NO_SURFACE) {
        printf("Failed to create EGL surface\n");
        switch_native_window_destroy(&native_win);
        return -1;
    }
    printf("EGL surface created\n");
    
    // Create EGL context
    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT) {
        printf("Failed to create EGL context\n");
        switch_native_window_destroy(&native_win);
        return -1;
    }
    printf("EGL context created\n");
    
    // Make current
    if (!eglMakeCurrent(display, surface, surface, context)) {
        printf("Failed to make EGL context current\n");
        switch_native_window_destroy(&native_win);
        return -1;
    }
    printf("EGL context made current\n");
    
    // Test basic OpenGL ES 2.0 functionality
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f); // Dark blue color
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Swap buffers
    if (!eglSwapBuffers(display, surface)) {
        printf("Failed to swap buffers\n");
    } else {
        printf("Buffers swapped successfully - you should see a dark blue screen!\n");
    }
    
    // Cleanup
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
    switch_native_window_destroy(&native_win);
    
    printf("VitaNS: Mesa EGL test completed successfully!\n");
    
    // Keep console open for a moment to see output
    PadState pad;
    padInitializeDefault(&pad);
    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus) {
            break;
        }
        consoleUpdate(nullptr);
    }
    
    consoleExit(nullptr);
    return 0;
}
