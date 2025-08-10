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
    
    // Initialize input
    if (R_FAILED(hidInitialize())) {
        return -1;
    }
    
    // Wait for input to exit
    while (appletMainLoop()) {
        hidScanInput();
        if (hidKeysDown(HidControllerID_Player1) & HidNpadButton_Plus) {
            break;
        }
        svcSleepThread(10000000); // 10ms
    }
    
    // Cleanup
    hidExit();
    viExit();
    smExit();
    
    return 0;
}
