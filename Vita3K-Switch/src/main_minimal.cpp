#include <switch.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

// Global log file for minimal test
static std::ofstream g_logFile;

// Initialize logging to SD card
static void initializeLogging() {
    mkdir("sdmc:/switch", 0777);
    mkdir("sdmc:/switch/vitans", 0777);
    mkdir("sdmc:/switch/vitans/logs", 0777);
    
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "sdmc:/switch/vitans/logs/vitans_minimal_" 
       << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") 
       << ".log";
    
    g_logFile.open(ss.str(), std::ios::out | std::ios::app);
}

#define LOG(msg, ...) do { \
    char buffer[1024]; \
    snprintf(buffer, sizeof(buffer), msg, ##__VA_ARGS__); \
    printf("%s\n", buffer); \
    if (g_logFile.is_open()) { \
        g_logFile << buffer << std::endl; \
        g_logFile.flush(); \
    } \
} while(0)

// Minimal test version - test basic functionality without EGL first

int main(int argc, char* argv[]) {
    // Initialize console for debug output
    consoleInit(nullptr);
    initializeLogging();
    
    LOG("=== VitaNS Minimal Test ===");
    LOG("Testing basic Switch functionality...");
    LOG("This version does NOT use EGL/Mesa - just basic Switch APIs");
    
    // Test 1: Basic memory allocation
    LOG("Test 1: Memory allocation...");
    void* test_mem = malloc(1024);
    if (test_mem) {
        LOG("SUCCESS: malloc worked");
        memset(test_mem, 0xAA, 1024);
        LOG("SUCCESS: memset worked");
        free(test_mem);
        LOG("SUCCESS: free worked");
    } else {
        LOG("FAILED: malloc failed");
    }
    
    // Test 2: Basic Switch services
    LOG("Test 2: Switch services...");
    AppletOperationMode mode = appletGetOperationMode();
    LOG("Operation mode: %s", 
           mode == AppletOperationMode_Console ? "Docked" : "Handheld");
    
    // Test 3: Time/sleep functions
    LOG("Test 3: Sleep function...");
    LOG("Sleeping for 1 second...");
    svcSleepThread(1000000000); // 1 second
    LOG("SUCCESS: Sleep completed");
    
    // Test 4: Input initialization
    LOG("Test 4: Input system...");
    PadState pad;
    padInitializeDefault(&pad);
    LOG("SUCCESS: Pad initialized");
    
    LOG("=== All basic tests passed! ===");
    LOG("If you see this in the log file, basic Switch functionality works!");
    LOG("Press [+] to exit...");
    
    // Wait for input
    while (appletMainLoop()) {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);
        if (kDown & HidNpadButton_Plus) {
            LOG("Plus button pressed - exiting");
            break;
        }
        consoleUpdate(nullptr);
        svcSleepThread(16666667); // ~60 FPS
    }
    
    LOG("Minimal test completed successfully!");
    if (g_logFile.is_open()) {
        g_logFile.close();
    }
    consoleExit(nullptr);
    return 0;
}
