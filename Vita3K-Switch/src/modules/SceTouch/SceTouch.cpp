#include "SceTouch.h"
#include "../module.h"
#include "../../core/emulator/emulator.h"
#include <cstdio>
#include <cstring>
#include <chrono>
#include <algorithm>

// PS Vita touch panel dimensions
#define VITA_FRONT_TOUCH_WIDTH 1920
#define VITA_FRONT_TOUCH_HEIGHT 1080
#define VITA_BACK_TOUCH_WIDTH 1920
#define VITA_BACK_TOUCH_HEIGHT 1080

// Switch touch panel dimensions
#define SWITCH_TOUCH_WIDTH 1280
#define SWITCH_TOUCH_HEIGHT 720

// Stub implementation to satisfy pure virtual requirement
void SceTouch::registerFunctions() {
    // Register system calls (already done in constructor for now)
    // Add additional registration logic here if needed
}

SceTouch::SceTouch() 
    : Module("SceTouch")
    , frontSamplingState(SCE_TOUCH_SAMPLING_STATE_STOP)
    , backSamplingState(SCE_TOUCH_SAMPLING_STATE_STOP)
{
    // Register system calls with wrapper functions
    registerFunction("sceTouchGetPanelInfo", 0xBAD73CDB, 
        [](Emulator& emu, uint32_t nid, const std::vector<uint32_t>& args) -> int {
            auto instance = std::static_pointer_cast<SceTouch>(emu.getModuleManager().findModule("SceTouch"));
            if (!instance) return -1;
            return instance->sceTouchGetPanelInfo(static_cast<uint8_t>(args[0]), 
                                                  reinterpret_cast<SceTouchPanelInfo*>(args[1]));
        }, 2);
    
    registerFunction("sceTouchRead", 0x169A1D58,
        [](Emulator& emu, uint32_t nid, const std::vector<uint32_t>& args) -> int {
            auto instance = std::static_pointer_cast<SceTouch>(emu.getModuleManager().findModule("SceTouch"));
            if (!instance) return -1;
            return instance->sceTouchRead(static_cast<uint8_t>(args[0]),
                                        reinterpret_cast<SceTouchReport*>(args[1]),
                                        args[2]);
        }, 3);
    
    registerFunction("sceTouchPeek", 0xFF082DF0,
        [](Emulator& emu, uint32_t nid, const std::vector<uint32_t>& args) -> int {
            auto instance = std::static_pointer_cast<SceTouch>(emu.getModuleManager().findModule("SceTouch"));
            if (!instance) return -1;
            return instance->sceTouchPeek(static_cast<uint8_t>(args[0]),
                                        reinterpret_cast<SceTouchReport*>(args[1]),
                                        args[2]);
        }, 3);
    
    registerFunction("sceTouchSetSamplingState", 0xE3BBCBF7,
        [](Emulator& emu, uint32_t nid, const std::vector<uint32_t>& args) -> int {
            auto instance = std::static_pointer_cast<SceTouch>(emu.getModuleManager().findModule("SceTouch"));
            if (!instance) return -1;
            return instance->sceTouchSetSamplingState(static_cast<uint8_t>(args[0]), args[1]);
        }, 2);
    
    registerFunction("sceTouchGetSamplingState", 0x1B9C5D14,
        [](Emulator& emu, uint32_t nid, const std::vector<uint32_t>& args) -> int {
            auto instance = std::static_pointer_cast<SceTouch>(emu.getModuleManager().findModule("SceTouch"));
            if (!instance) return -1;
            return instance->sceTouchGetSamplingState(static_cast<uint8_t>(args[0]),
                                                     reinterpret_cast<uint32_t*>(args[1]));
        }, 2);
    
    // Initialize panel info
    initializePanelInfo();
}

SceTouch::~SceTouch() {
    finalize();
}

bool SceTouch::initialize() {
    printf("[SceTouch] Initializing touch module\n");
    
    // Initialize touch data
    frontTouch.timestamp = 0;
    frontTouch.reports.clear();
    
    backTouch.timestamp = 0;
    backTouch.reports.clear();
    
    // Start in sampling state
    frontSamplingState = SCE_TOUCH_SAMPLING_STATE_START;
    backSamplingState = SCE_TOUCH_SAMPLING_STATE_START;
    
    printf("[SceTouch] Touch module initialized successfully\n");
    return true;
}

void SceTouch::finalize() {
    printf("[SceTouch] Finalizing touch module\n");
    
    // Clear touch data
    std::lock_guard<std::mutex> lock(touchMutex);
    frontTouch.reports.clear();
    backTouch.reports.clear();
    
    // Stop sampling
    frontSamplingState = SCE_TOUCH_SAMPLING_STATE_STOP;
    backSamplingState = SCE_TOUCH_SAMPLING_STATE_STOP;
}

void SceTouch::initializePanelInfo() {
    // Front panel info
    frontPanelInfo.minAaX = 0;
    frontPanelInfo.minAaY = 0;
    frontPanelInfo.maxAaX = 100;
    frontPanelInfo.maxAaY = 100;
    frontPanelInfo.minDispX = 0;
    frontPanelInfo.minDispY = 0;
    frontPanelInfo.maxDispX = 100;
    frontPanelInfo.maxDispY = 100;
    frontPanelInfo.resolutionX = VITA_FRONT_TOUCH_WIDTH;
    frontPanelInfo.resolutionY = VITA_FRONT_TOUCH_HEIGHT;
    memset(frontPanelInfo.reserved, 0, sizeof(frontPanelInfo.reserved));
    
    // Back panel info
    backPanelInfo.minAaX = 0;
    backPanelInfo.minAaY = 0;
    backPanelInfo.maxAaX = 100;
    backPanelInfo.maxAaY = 100;
    backPanelInfo.minDispX = 0;
    backPanelInfo.minDispY = 0;
    backPanelInfo.maxDispX = 100;
    backPanelInfo.maxDispY = 100;
    backPanelInfo.resolutionX = VITA_BACK_TOUCH_WIDTH;
    backPanelInfo.resolutionY = VITA_BACK_TOUCH_HEIGHT;
    memset(backPanelInfo.reserved, 0, sizeof(backPanelInfo.reserved));
}

void SceTouch::updateTouchState(bool touched, float x, float y) {
    std::lock_guard<std::mutex> lock(touchMutex);
    
    // Only update front touch for now (Switch only has front touch panel)
    if (frontSamplingState != SCE_TOUCH_SAMPLING_STATE_START) {
        return;
    }
    
    // Get current timestamp
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    
    // Update front touch data
    frontTouch.timestamp = timestamp;
    
    if (touched) {
        // Convert Switch touch coordinates to PS Vita coordinates
        float vitaX = (x / SWITCH_TOUCH_WIDTH) * VITA_FRONT_TOUCH_WIDTH;
        float vitaY = (y / SWITCH_TOUCH_HEIGHT) * VITA_FRONT_TOUCH_HEIGHT;
        
        // Check if we already have a touch report
        if (frontTouch.reports.empty()) {
            // Create a new touch report
            SceTouchReport report;
            report.id = 0;
            report.force = 128; // Medium force
            report.reserved = 0;
            report.x = vitaX;
            report.y = vitaY;
            
            frontTouch.reports.push_back(report);
        } else {
            // Update existing touch report
            frontTouch.reports[0].x = vitaX;
            frontTouch.reports[0].y = vitaY;
        }
    } else {
        // Clear touch reports when not touched
        frontTouch.reports.clear();
    }
}

int SceTouch::sceTouchGetPanelInfo(uint8_t port, SceTouchPanelInfo *pPanelInfo) {
    printf("[SceTouch] sceTouchGetPanelInfo called: port=%d\n", port);
    
    if (!pPanelInfo) {
        printf("[SceTouch] Error: pPanelInfo is NULL\n");
        return -1;
    }
    
    if (port != SCE_TOUCH_PORT_FRONT && port != SCE_TOUCH_PORT_BACK) {
        printf("[SceTouch] Error: Invalid port %d\n", port);
        return -1;
    }
    
    // Copy panel info
    if (port == SCE_TOUCH_PORT_FRONT) {
        memcpy(pPanelInfo, &frontPanelInfo, sizeof(SceTouchPanelInfo));
    } else {
        memcpy(pPanelInfo, &backPanelInfo, sizeof(SceTouchPanelInfo));
    }
    
    return 0;
}

int SceTouch::sceTouchRead(uint8_t port, SceTouchReport *pData, uint32_t nBufs) {
    if (!pData) {
        printf("[SceTouch] Error: pData is NULL\n");
        return -1;
    }
    
    if (port != SCE_TOUCH_PORT_FRONT && port != SCE_TOUCH_PORT_BACK) {
        printf("[SceTouch] Error: Invalid port %d\n", port);
        return -1;
    }
    
    if (nBufs > SCE_TOUCH_MAX_REPORT) {
        printf("[SceTouch] Warning: nBufs %d exceeds maximum %d\n", nBufs, SCE_TOUCH_MAX_REPORT);
        nBufs = SCE_TOUCH_MAX_REPORT;
    }
    
    std::lock_guard<std::mutex> lock(touchMutex);
    
    // Check sampling state
    if ((port == SCE_TOUCH_PORT_FRONT && frontSamplingState != SCE_TOUCH_SAMPLING_STATE_START) ||
        (port == SCE_TOUCH_PORT_BACK && backSamplingState != SCE_TOUCH_SAMPLING_STATE_START)) {
        return 0;
    }
    
    // Get touch data
    TouchData& touchData = (port == SCE_TOUCH_PORT_FRONT) ? frontTouch : backTouch;
    
    // Copy reports
    uint32_t reportCount = std::min(static_cast<uint32_t>(touchData.reports.size()), nBufs);
    
    if (reportCount > 0) {
        memcpy(pData, touchData.reports.data(), reportCount * sizeof(SceTouchReport));
        
        // Clear reports after reading
        touchData.reports.clear();
    }
    
    return reportCount;
}

int SceTouch::sceTouchPeek(uint8_t port, SceTouchReport *pData, uint32_t nBufs) {
    if (!pData) {
        printf("[SceTouch] Error: pData is NULL\n");
        return -1;
    }
    
    if (port != SCE_TOUCH_PORT_FRONT && port != SCE_TOUCH_PORT_BACK) {
        printf("[SceTouch] Error: Invalid port %d\n", port);
        return -1;
    }
    
    if (nBufs > SCE_TOUCH_MAX_REPORT) {
        printf("[SceTouch] Warning: nBufs %d exceeds maximum %d\n", nBufs, SCE_TOUCH_MAX_REPORT);
        nBufs = SCE_TOUCH_MAX_REPORT;
    }
    
    std::lock_guard<std::mutex> lock(touchMutex);
    
    // Check sampling state
    if ((port == SCE_TOUCH_PORT_FRONT && frontSamplingState != SCE_TOUCH_SAMPLING_STATE_START) ||
        (port == SCE_TOUCH_PORT_BACK && backSamplingState != SCE_TOUCH_SAMPLING_STATE_START)) {
        return 0;
    }
    
    // Get touch data
    TouchData& touchData = (port == SCE_TOUCH_PORT_FRONT) ? frontTouch : backTouch;
    
    // Copy reports (peek doesn't clear reports)
    uint32_t reportCount = std::min(static_cast<uint32_t>(touchData.reports.size()), nBufs);
    
    if (reportCount > 0) {
        memcpy(pData, touchData.reports.data(), reportCount * sizeof(SceTouchReport));
    }
    
    return reportCount;
}

int SceTouch::sceTouchSetSamplingState(uint8_t port, uint32_t state) {
    printf("[SceTouch] sceTouchSetSamplingState called: port=%d, state=%d\n", port, state);
    
    if (port != SCE_TOUCH_PORT_FRONT && port != SCE_TOUCH_PORT_BACK) {
        printf("[SceTouch] Error: Invalid port %d\n", port);
        return -1;
    }
    
    if (state != SCE_TOUCH_SAMPLING_STATE_STOP && state != SCE_TOUCH_SAMPLING_STATE_START) {
        printf("[SceTouch] Error: Invalid state %d\n", state);
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(touchMutex);
    
    // Set sampling state
    if (port == SCE_TOUCH_PORT_FRONT) {
        frontSamplingState = state;
    } else {
        backSamplingState = state;
    }
    
    return 0;
}

int SceTouch::sceTouchGetSamplingState(uint8_t port, uint32_t *pState) {
    printf("[SceTouch] sceTouchGetSamplingState called: port=%d\n", port);
    
    if (!pState) {
        printf("[SceTouch] Error: pState is NULL\n");
        return -1;
    }
    
    if (port != SCE_TOUCH_PORT_FRONT && port != SCE_TOUCH_PORT_BACK) {
        printf("[SceTouch] Error: Invalid port %d\n", port);
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(touchMutex);
    
    // Get sampling state
    if (port == SCE_TOUCH_PORT_FRONT) {
        *pState = frontSamplingState;
    } else {
        *pState = backSamplingState;
    }
    
    return 0;
}
