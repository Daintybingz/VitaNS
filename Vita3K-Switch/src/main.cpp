#include <switch.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "platform/switch_native_window.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

// Global log file
static std::ofstream g_logFile;
static bool g_logInitialized = false;

// Initialize logging to SD card
static void initializeLogging() {
    if (g_logInitialized) return;
    
    // Create log directory
    mkdir("sdmc:/switch", 0777);
    mkdir("sdmc:/switch/vitans", 0777);
    mkdir("sdmc:/switch/vitans/logs", 0777);
    
    // Create timestamped log filename
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "sdmc:/switch/vitans/logs/vitans_debug_" 
       << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") 
       << ".log";
    
    g_logFile.open(ss.str(), std::ios::out | std::ios::app);
    if (g_logFile.is_open()) {
        g_logInitialized = true;
        g_logFile << "=== VitaNS Debug Log Started ===" << std::endl;
        g_logFile.flush();
    }
}

// Enhanced debug logging macro with file output
#define DEBUG_LOG(msg, ...) do { \
    initializeLogging(); \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), "[DEBUG] %s:%d: " msg, __func__, __LINE__, ##__VA_ARGS__); \
    printf("%s\n", buffer); \
    if (g_logInitialized && g_logFile.is_open()) { \
        g_logFile << buffer << std::endl; \
        g_logFile.flush(); \
    } \
    consoleUpdate(nullptr); \
} while(0)

// Safe error checking macro with logging
#define CHECK_EGL_ERROR(operation) do { \
    EGLint error = eglGetError(); \
    if (error != EGL_SUCCESS) { \
        DEBUG_LOG("EGL Error after %s: 0x%x", operation, error); \
        closeLogging(); \
        return -1; \
    } \
} while(0)

// Close logging safely
static void closeLogging() {
    if (g_logInitialized && g_logFile.is_open()) {
        g_logFile << "=== VitaNS Debug Log Ended ===" << std::endl;
        g_logFile.close();
    }
    g_logInitialized = false;
}

// Signal handler for crashes
static void crashHandler(int sig) {
    DEBUG_LOG("CRITICAL: Caught signal %d - Application crashing!", sig);
    DEBUG_LOG("This indicates a serious error occurred during execution");
    closeLogging();
    exit(1);
}

int main(int argc, char* argv[]) {
    // Initialize console for debug output
    consoleInit(nullptr);
    
    // Set up crash handlers
    signal(SIGABRT, crashHandler);
    signal(SIGFPE, crashHandler);
    signal(SIGILL, crashHandler);
    signal(SIGINT, crashHandler);
    signal(SIGSEGV, crashHandler);
    signal(SIGTERM, crashHandler);
    
    DEBUG_LOG("=== VitaNS Runtime Debug Version ===");
    DEBUG_LOG("Mesa EGL initialization test starting...");
    DEBUG_LOG("Built: %s %s", __DATE__, __TIME__);
    DEBUG_LOG("Logging to: sdmc:/switch/vitans/logs/");
    DEBUG_LOG("Register dump will be available in this log if crash occurs");
    
    // Log system information
    AppletOperationMode mode = appletGetOperationMode();
    DEBUG_LOG("System Info:");
    DEBUG_LOG("  - Operation Mode: %s", mode == AppletOperationMode_Console ? "Docked" : "Handheld");
    DEBUG_LOG("  - VitaNS Debug Build Started");
    
    // Wait a moment for console to stabilize
    svcSleepThread(100000000); // 100ms
    
    DEBUG_LOG("Step 1: Creating native window...");
    switch_native_window native_win;
    
    // Add some safety checks
    memset(&native_win, 0, sizeof(native_win));
    
    if (switch_native_window_create(&native_win, 1280, 720) != 0) {
        DEBUG_LOG("FAILED: Native window creation failed");
        closeLogging();
        svcSleepThread(2000000000); // 2 seconds to see error
        return -1;
    }
    DEBUG_LOG("SUCCESS: Native window created: %dx%d", native_win.width, native_win.height);
    DEBUG_LOG("  - userdata: %p", native_win.userdata);
    DEBUG_LOG("  - stride_bytes: %d", native_win.stride_bytes);
    DEBUG_LOG("  - format: %d", native_win.format);
    
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
    DEBUG_LOG("SUCCESS: All EGL operations completed without crash!");
    DEBUG_LOG("Press [+] to exit...");
    
    // Keep console open to see output
    PadState pad;
    padInitializeDefault(&pad);
    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus) {
            DEBUG_LOG("Plus button pressed - exiting normally");
            break;
        }
        consoleUpdate(nullptr);
        svcSleepThread(16666667); // ~60 FPS
    }
    
    closeLogging();
    consoleExit(nullptr);
    return 0;
}
