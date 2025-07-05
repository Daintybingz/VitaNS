#pragma once

#include "../module.h"
#include <vector>
#include <mutex>
#include <cstdint>

// PS Vita touch panel port definitions
#define SCE_TOUCH_PORT_FRONT 0
#define SCE_TOUCH_PORT_BACK 1
#define SCE_TOUCH_MAX_REPORT 8

// PS Vita touch sampling state
enum SceTouchSamplingState {
    SCE_TOUCH_SAMPLING_STATE_STOP,
    SCE_TOUCH_SAMPLING_STATE_START
};

// PS Vita touch report info
struct SceTouchReport {
    uint8_t id;          // Touch ID
    uint8_t force;       // Touch force
    uint16_t reserved;   // Reserved data
    float x;             // X position
    float y;             // Y position
};

// PS Vita touch panel info
struct SceTouchPanelInfo {
    uint8_t minAaX;      // Min active area X
    uint8_t minAaY;      // Min active area Y
    uint8_t maxAaX;      // Max active area X
    uint8_t maxAaY;      // Max active area Y
    uint8_t minDispX;    // Min display X
    uint8_t minDispY;    // Min display Y
    uint8_t maxDispX;    // Max display X
    uint8_t maxDispY;    // Max display Y
    uint16_t resolutionX; // Resolution X
    uint16_t resolutionY; // Resolution Y
    uint8_t reserved[8]; // Reserved data
};

// Touch data structure
struct TouchData {
    uint64_t timestamp;
    std::vector<SceTouchReport> reports;
};

class SceTouch : public Module {
public:
    SceTouch();
    ~SceTouch();
    
    // Initialize and finalize
    bool initialize();
    void finalize();
    
    // Update touch state from Switch touch input
    void updateTouchState(bool touched, float x, float y);
    
    // PS Vita system calls
    int sceTouchGetPanelInfo(uint8_t port, SceTouchPanelInfo *pPanelInfo);
    int sceTouchRead(uint8_t port, SceTouchReport *pData, uint32_t nBufs);
    int sceTouchPeek(uint8_t port, SceTouchReport *pData, uint32_t nBufs);
    int sceTouchSetSamplingState(uint8_t port, uint32_t state);
    int sceTouchGetSamplingState(uint8_t port, uint32_t *pState);
    
public:
    void registerFunctions() override;
private:
    // Touch state
    TouchData frontTouch;
    TouchData backTouch;
    uint32_t frontSamplingState;
    uint32_t backSamplingState;
    
    // Thread safety
    std::mutex touchMutex;
    
    // Panel info
    SceTouchPanelInfo frontPanelInfo;
    SceTouchPanelInfo backPanelInfo;
    
    // Initialize panel info
    void initializePanelInfo();
};
