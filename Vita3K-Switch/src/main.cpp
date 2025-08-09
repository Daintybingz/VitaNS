#include <switch.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "platform/switch_native_window.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Debug logging macro
#define DEBUG_LOG(msg, ...) do { \
    printf("[DEBUG] %s:%d: " msg "\n", __func__, __LINE__, ##__VA_ARGS__); \
    consoleUpdate(nullptr); \
} while(0)

// Safe error checking macro
#define CHECK_EGL_ERROR(operation) do { \
    EGLint error = eglGetError(); \
    if (error != EGL_SUCCESS) { \
        DEBUG_LOG("EGL Error after %s: 0x%x", operation, error); \
        return -1; \
    } \
} while(0)

int main(int argc, char* argv[]) {
    // Initialize console for debug output
    consoleInit(nullptr);
    DEBUG_LOG("=== VitaNS Runtime Debug Version ===");
    DEBUG_LOG("Mesa EGL initialization test starting...");
    DEBUG_LOG("Built: %s %s", __DATE__, __TIME__);
    
    // Wait a moment for console to stabilize
    svcSleepThread(100000000); // 100ms
    
    DEBUG_LOG("Step 1: Creating native window...");
    switch_native_window native_win;
    
    // Add some safety checks
    memset(&native_win, 0, sizeof(native_win));
    
    if (switch_native_window_create(&native_win, 1280, 720) != 0) {
        DEBUG_LOG("FAILED: Native window creation failed");
        svcSleepThread(2000000000); // 2 seconds to see error
        return -1;
    }
    DEBUG_LOG("SUCCESS: Native window created: %dx%d", native_win.width, native_win.height);
    DEBUG_LOG("  - framebuffer_ptr: %p", native_win.framebuffer_ptr);
    DEBUG_LOG("  - stride_bytes: %u", native_win.stride_bytes);
    
    DEBUG_LOG("Step 2: Getting EGL display...");
    // Clear any previous EGL errors
    while (eglGetError() != EGL_SUCCESS);
    
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    CHECK_EGL_ERROR("eglGetDisplay");
    
    if (display == EGL_NO_DISPLAY) {
        DEBUG_LOG("FAILED: eglGetDisplay returned EGL_NO_DISPLAY");
        switch_native_window_destroy(&native_win);
        svcSleepThread(2000000000);
        return -1;
    }
    DEBUG_LOG("SUCCESS: EGL display obtained: %p", display);
    
    DEBUG_LOG("Step 3: Initializing EGL...");
    EGLint major, minor;
    EGLBoolean init_result = eglInitialize(display, &major, &minor);
    EGLint init_error = eglGetError();
    
    if (!init_result) {
        DEBUG_LOG("FAILED: eglInitialize returned EGL_FALSE");
        DEBUG_LOG("EGL Error: 0x%x", init_error);
        switch_native_window_destroy(&native_win);
        svcSleepThread(3000000000); // 3 seconds to see error
        return -1;
    }
    DEBUG_LOG("SUCCESS: EGL initialized: version %d.%d", major, minor);
    DEBUG_LOG("EGL Vendor: %s", eglQueryString(display, EGL_VENDOR));
    DEBUG_LOG("EGL Version: %s", eglQueryString(display, EGL_VERSION));
    
    DEBUG_LOG("Step 4: Choosing EGL config...");
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
    EGLBoolean config_result = eglChooseConfig(display, config_attribs, &config, 1, &num_configs);
    CHECK_EGL_ERROR("eglChooseConfig");
    
    if (!config_result || num_configs == 0) {
        DEBUG_LOG("FAILED: eglChooseConfig - result: %s, configs found: %d", 
                  config_result ? "true" : "false", num_configs);
        eglTerminate(display);
        switch_native_window_destroy(&native_win);
        svcSleepThread(2000000000);
        return -1;
    }
    DEBUG_LOG("SUCCESS: EGL config chosen (%d configs available)", num_configs);
    
    DEBUG_LOG("Step 5: Creating EGL surface...");
    EGLSurface surface = eglCreateWindowSurface(display, config, 
                                               (EGLNativeWindowType)&native_win, nullptr);
    CHECK_EGL_ERROR("eglCreateWindowSurface");
    
    if (surface == EGL_NO_SURFACE) {
        DEBUG_LOG("FAILED: eglCreateWindowSurface returned EGL_NO_SURFACE");
        eglTerminate(display);
        switch_native_window_destroy(&native_win);
        svcSleepThread(2000000000);
        return -1;
    }
    DEBUG_LOG("SUCCESS: EGL surface created: %p", surface);
    
    DEBUG_LOG("Step 6: Creating EGL context...");
    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);
    CHECK_EGL_ERROR("eglCreateContext");
    
    if (context == EGL_NO_CONTEXT) {
        DEBUG_LOG("FAILED: eglCreateContext returned EGL_NO_CONTEXT");
        eglDestroySurface(display, surface);
        eglTerminate(display);
        switch_native_window_destroy(&native_win);
        svcSleepThread(2000000000);
        return -1;
    }
    DEBUG_LOG("SUCCESS: EGL context created: %p", context);
    
    DEBUG_LOG("Step 7: Making EGL context current...");
    EGLBoolean make_current_result = eglMakeCurrent(display, surface, surface, context);
    CHECK_EGL_ERROR("eglMakeCurrent");
    
    if (!make_current_result) {
        DEBUG_LOG("FAILED: eglMakeCurrent returned EGL_FALSE");
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
        switch_native_window_destroy(&native_win);
        svcSleepThread(2000000000);
        return -1;
    }
    DEBUG_LOG("SUCCESS: EGL context made current");
    
    DEBUG_LOG("Step 8: Testing OpenGL ES operations...");
    
    // Test OpenGL ES calls with error checking
    DEBUG_LOG("Setting clear color...");
    glClearColor(0.1f, 0.2f, 0.4f, 1.0f); // Dark blue color
    GLenum gl_error = glGetError();
    if (gl_error != GL_NO_ERROR) {
        DEBUG_LOG("FAILED: glClearColor error: 0x%x", gl_error);
    } else {
        DEBUG_LOG("SUCCESS: glClearColor set");
    }
    
    DEBUG_LOG("Clearing screen...");
    glClear(GL_COLOR_BUFFER_BIT);
    gl_error = glGetError();
    if (gl_error != GL_NO_ERROR) {
        DEBUG_LOG("FAILED: glClear error: 0x%x", gl_error);
    } else {
        DEBUG_LOG("SUCCESS: Screen cleared");
    }
    
    DEBUG_LOG("Step 9: Swapping buffers...");
    EGLBoolean swap_result = eglSwapBuffers(display, surface);
    CHECK_EGL_ERROR("eglSwapBuffers");
    
    if (!swap_result) {
        DEBUG_LOG("FAILED: eglSwapBuffers returned EGL_FALSE");
    } else {
        DEBUG_LOG("SUCCESS: Buffers swapped - you should see a dark blue screen!");
    }
    
    DEBUG_LOG("Step 10: Cleanup...");
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
    switch_native_window_destroy(&native_win);
    
    DEBUG_LOG("=== VitaNS Mesa EGL test completed successfully! ===");
    DEBUG_LOG("Press [+] to exit...");
    
    // Keep console open to see output
    PadState pad;
    padInitializeDefault(&pad);
    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus) {
            break;
        }
        consoleUpdate(nullptr);
        svcSleepThread(16666667); // ~60 FPS
    }
    
    consoleExit(nullptr);
    return 0;
}
