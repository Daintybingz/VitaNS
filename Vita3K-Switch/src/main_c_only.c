#include <switch.h>

int main(int argc, char* argv[]) {
    // Initialize Switch services
    if (R_FAILED(smInitialize())) {
        return -1;
    }
    
    // Initialize video
    if (R_FAILED(viInitialize(ViServiceType_Manager))) {
        return -1;
    }
    
    // Initialize input using modern Switch HID API
    PadState pad;
    padInitializeDefault(&pad);
    
    // Wait for input to exit
    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus) {
            break;
        }
        svcSleepThread(10000000); // 10ms
    }
    
    // Cleanup
    viExit();
    smExit();
    
    return 0;
}
