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
    
    // Just wait a bit and exit - no input handling
    svcSleepThread(1000000000); // 1 second
    
    // Cleanup
    viExit();
    smExit();
    
    return 0;
}
