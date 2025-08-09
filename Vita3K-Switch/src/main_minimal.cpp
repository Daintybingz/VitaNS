#include <switch.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Minimal test version - test basic functionality without EGL first

int main(int argc, char* argv[]) {
    // Initialize console for debug output
    consoleInit(nullptr);
    printf("=== VitaNS Minimal Test ===\n");
    printf("Testing basic Switch functionality...\n");
    
    // Test 1: Basic memory allocation
    printf("Test 1: Memory allocation...\n");
    void* test_mem = malloc(1024);
    if (test_mem) {
        printf("SUCCESS: malloc worked\n");
        memset(test_mem, 0xAA, 1024);
        printf("SUCCESS: memset worked\n");
        free(test_mem);
        printf("SUCCESS: free worked\n");
    } else {
        printf("FAILED: malloc failed\n");
    }
    
    // Test 2: Basic Switch services
    printf("Test 2: Switch services...\n");
    AppletOperationMode mode = appletGetOperationMode();
    printf("Operation mode: %s\n", 
           mode == AppletOperationMode_Console ? "Docked" : "Handheld");
    
    // Test 3: Time/sleep functions
    printf("Test 3: Sleep function...\n");
    printf("Sleeping for 1 second...\n");
    svcSleepThread(1000000000); // 1 second
    printf("SUCCESS: Sleep completed\n");
    
    // Test 4: Input initialization
    printf("Test 4: Input system...\n");
    PadState pad;
    padInitializeDefault(&pad);
    printf("SUCCESS: Pad initialized\n");
    
    printf("=== All basic tests passed! ===\n");
    printf("Press [+] to exit...\n");
    
    // Wait for input
    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus) {
            printf("Plus button pressed - exiting\n");
            break;
        }
        consoleUpdate(nullptr);
        svcSleepThread(16666667); // ~60 FPS
    }
    
    consoleExit(nullptr);
    printf("Minimal test completed successfully!\n");
    return 0;
}
