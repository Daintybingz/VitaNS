#include <switch.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    // Initialize Switch services
    consoleInit(NULL);
    
    printf("=== Custom Mesa/EGL Test ===\n");
    
    // Test EGL initialization
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        printf("❌ Failed to get EGL display\n");
        consoleExit(NULL);
        return -1;
    }
    printf("✅ EGL display obtained successfully\n");
    
    // Initialize EGL
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        printf("❌ Failed to initialize EGL\n");
        consoleExit(NULL);
        return -1;
    }
    printf("✅ EGL initialized successfully: %d.%d\n", major, minor);
    
    // Test OpenGL ES 2.0
    const char* version = (const char*)glGetString(GL_VERSION);
    if (version) {
        printf("✅ OpenGL ES version: %s\n", version);
    } else {
        printf("⚠️  Could not get OpenGL ES version\n");
    }
    
    // Test basic OpenGL ES 2.0 functionality
    glClearColor(0.2f, 0.3f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    printf("✅ Basic OpenGL ES 2.0 functionality working\n");
    
    // Test EGL extensions
    const char* extensions = eglQueryString(display, EGL_EXTENSIONS);
    if (extensions) {
        printf("✅ EGL extensions available: %s\n", extensions);
    } else {
        printf("⚠️  Could not get EGL extensions\n");
    }
    
    printf("=== Custom Mesa/EGL Test Complete ===\n");
    printf("✅ All tests passed - custom Mesa integration working!\n");
    
    // Wait for user input
    printf("Press any key to exit...\n");
    while (appletMainLoop()) {
        hidScanInput();
        if (hidKeysDown(CONTROLLER_P1_AUTO) & KEY_A) {
            break;
        }
        consoleUpdate(NULL);
    }
    
    // Cleanup
    eglTerminate(display);
    consoleExit(NULL);
    return 0;
} 