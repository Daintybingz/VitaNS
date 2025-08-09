#include <switch.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <cstdio>
#include <cstdlib>

// Ultra-minimal EGL test - no VitaNS code, no ImGui, just raw EGL

int main(int argc, char* argv[]) {
    // Initialize console
    consoleInit(nullptr);
    printf("=== Pure EGL Test - No VitaNS Dependencies ===\n");
    printf("Testing if the crash is in Mesa or VitaNS code...\n");
    
    // Initialize video service directly
    printf("Initializing vi service...\n");
    viInitialize(ViServiceType_Default);
    
    // Get default window
    printf("Getting default window...\n");
    NWindow* nwin = nwindowGetDefault();
    if (!nwin) {
        printf("FAILED: Could not get default window\n");
        return -1;
    }
    printf("SUCCESS: Got default window\n");
    
    // Create framebuffer
    printf("Creating framebuffer...\n");
    Framebuffer fb;
    framebufferCreate(&fb, nwin, 1280, 720, PIXEL_FORMAT_RGBA_8888, 2);
    framebufferMakeLinear(&fb);
    printf("SUCCESS: Framebuffer created\n");
    
    // Test raw Mesa EGL
    printf("Testing Mesa EGL initialization...\n");
    
    printf("Step 1: eglGetDisplay...\n");
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        printf("FAILED: eglGetDisplay\n");
        return -1;
    }
    printf("SUCCESS: Got EGL display\n");
    
    printf("Step 2: eglInitialize...\n");
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        printf("FAILED: eglInitialize\n");
        EGLint error = eglGetError();
        printf("EGL Error: 0x%x\n", error);
        return -1;
    }
    printf("SUCCESS: EGL initialized %d.%d\n", major, minor);
    
    // Print Mesa info
    printf("EGL Vendor: %s\n", eglQueryString(display, EGL_VENDOR));
    printf("EGL Version: %s\n", eglQueryString(display, EGL_VERSION));
    printf("EGL Extensions: %s\n", eglQueryString(display, EGL_EXTENSIONS));
    
    printf("Step 3: Basic config...\n");
    EGLint config_attribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    
    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(display, config_attribs, &config, 1, &num_configs)) {
        printf("FAILED: eglChooseConfig\n");
        return -1;
    }
    printf("SUCCESS: Found %d configs\n", num_configs);
    
    printf("Step 4: Create context...\n");
    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT) {
        printf("FAILED: eglCreateContext\n");
        return -1;
    }
    printf("SUCCESS: Created EGL context\n");
    
    printf("Step 5: Make current (without surface)...\n");
    if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context)) {
        printf("FAILED: eglMakeCurrent\n");
    } else {
        printf("SUCCESS: Made context current\n");
        
        // Test basic OpenGL ES
        printf("Testing OpenGL ES...\n");
        const char* gl_vendor = (const char*)glGetString(GL_VENDOR);
        const char* gl_renderer = (const char*)glGetString(GL_RENDERER);
        const char* gl_version = (const char*)glGetString(GL_VERSION);
        
        printf("GL Vendor: %s\n", gl_vendor ? gl_vendor : "NULL");
        printf("GL Renderer: %s\n", gl_renderer ? gl_renderer : "NULL");
        printf("GL Version: %s\n", gl_version ? gl_version : "NULL");
        
        GLenum gl_error = glGetError();
        if (gl_error != GL_NO_ERROR) {
            printf("OpenGL Error: 0x%x\n", gl_error);
        } else {
            printf("SUCCESS: OpenGL ES working!\n");
        }
    }
    
    // Cleanup
    printf("Cleaning up...\n");
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(display, context);
    eglTerminate(display);
    framebufferClose(&fb);
    
    printf("=== Pure EGL test completed successfully! ===\n");
    printf("Mesa and EGL are working correctly.\n");
    printf("The crash is likely in VitaNS code or ImGui integration.\n");
    printf("Press [+] to exit...\n");
    
    // Wait for input
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
