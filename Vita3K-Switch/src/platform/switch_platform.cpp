#include "switch_platform.h"
#include <switch.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

// Fallback stubs for input functions if not defined by libnx
#ifndef hidScanInput
static inline void hidScanInput() {}
#endif
#ifndef hidKeysHeld
static inline uint64_t hidKeysHeld(int) { return 0; }
#endif
#ifndef hidTouchCount
static inline uint32_t hidTouchCount() { return 0; }
#endif
#ifndef hidTouchRead
static inline void hidTouchRead(touchPosition*, uint32_t) {}
#endif

// Singleton instance
SwitchPlatform& SwitchPlatform::getInstance() {
    static SwitchPlatform instance;
    return instance;
}

SwitchPlatform::SwitchPlatform() 
    : initialized(false), 
      docked(false), 
      screenWidth(1280), 
      screenHeight(720), 
      performanceProfile(1),
      currentButtonState(0),
      previousButtonState(0),
      touchActive(false) {
}

SwitchPlatform::~SwitchPlatform() {
    if (initialized) {
        finalize();
    }
}

bool SwitchPlatform::initialize() {
    if (initialized) {
        return true; // Already initialized
    }
    
    // Initialize Switch services
    romfsInit();
    
    // Check if the console is docked
    AppletOperationMode mode = appletGetOperationMode();
    docked = (mode == AppletOperationMode_Console);
    
    // Set screen dimensions based on docked state
    if (docked) {
        screenWidth = 1920;
        screenHeight = 1080;
    } else {
        screenWidth = 1280;
        screenHeight = 720;
    }
    
    // Initialize button mappings
    initializeButtonMappings();
    
    // Create necessary directories
    createDirectory(getBasePath());
    createDirectory(getSaveDataPath());
    createDirectory(getGamePath());
    
    initialized = true;
    return true;
}

void SwitchPlatform::finalize() {
    if (!initialized) {
        return;
    }
    
    // Clean up Switch services
    romfsExit();
    
    initialized = false;
}

bool SwitchPlatform::isDocked() const {
    AppletOperationMode mode = appletGetOperationMode();
    return (mode == AppletOperationMode_Console);
}

int SwitchPlatform::getScreenWidth() const {
    return screenWidth;
}

int SwitchPlatform::getScreenHeight() const {
    return screenHeight;
}

uint64_t SwitchPlatform::getAvailableMemory() const {
    // Get available application memory
    // This is an approximation
    return 3ULL * 1024 * 1024 * 1024; // ~3GB
}

std::string SwitchPlatform::getSystemLanguage() const {
    uint64_t languageCode = 0;
    setGetSystemLanguage(&languageCode);
    
    // Map language code to language string
    switch (languageCode) {
        case 0:  return "ja";    // Japanese
        case 1:  return "en-US"; // American English
        case 2:  return "fr";    // French
        case 3:  return "de";    // German
        case 4:  return "it";    // Italian
        case 5:  return "es";    // Spanish
        case 6:  return "zh-CN"; // Chinese (Simplified)
        case 7:  return "ko";    // Korean
        case 8:  return "nl";    // Dutch
        case 9:  return "pt";    // Portuguese
        case 10: return "ru";    // Russian
        case 11: return "zh-TW"; // Chinese (Traditional)
        case 12: return "en-GB"; // British English
        case 13: return "fr-CA"; // Canadian French
        case 14: return "es-419"; // Latin American Spanish
        default: return "en-US"; // Default to American English
    }
}

std::string SwitchPlatform::getBasePath() const {
    return "sdmc:/switch/vitans/";
}

std::string SwitchPlatform::getSaveDataPath() const {
    return getBasePath() + "savedata/";
}

std::string SwitchPlatform::getGamePath() const {
    return getBasePath() + "games/";
}

bool SwitchPlatform::createDirectory(const std::string& path) const {
    DIR* dir = opendir(path.c_str());
    if (dir) {
        closedir(dir);
        return true; // Directory already exists
    }
    
    // Create directory
    int result = mkdir(path.c_str(), 0777);
    return (result == 0);
}

void SwitchPlatform::updateInputState() {
    // Update previous state
    previousButtonState = currentButtonState;
    
    // Get current button state
    hidScanInput();
    currentButtonState = hidKeysHeld((int)CONTROLLER_P1_AUTO);
    
    // Get touch state
    touchActive = false;
    if (hidTouchCount() > 0) {
        hidTouchRead(&touchPos, 0);
        touchActive = true;
    }
}

bool SwitchPlatform::isButtonPressed(uint32_t button) const {
    return (currentButtonState & button) && !(previousButtonState & button);
}

bool SwitchPlatform::isButtonHeld(uint32_t button) const {
    return (currentButtonState & button);
}

bool SwitchPlatform::isButtonReleased(uint32_t button) const {
    return !(currentButtonState & button) && (previousButtonState & button);
}

void SwitchPlatform::getTouchPosition(int& x, int& y) const {
    if (touchActive) {
        x = touchPos.px;
        y = touchPos.py;
    } else {
        x = -1;
        y = -1;
    }
}

bool SwitchPlatform::isTouchActive() const {
    return touchActive;
}

void SwitchPlatform::initializeButtonMappings() {
    // PS Vita to Switch button mappings
    // These are just examples and can be customized
    vitaToSwitchMap.clear();
    vitaToSwitchMap.push_back({SCE_CTRL_UP, KEY_DUP});
    vitaToSwitchMap.push_back({SCE_CTRL_DOWN, KEY_DDOWN});
    vitaToSwitchMap.push_back({SCE_CTRL_LEFT, KEY_DLEFT});
    vitaToSwitchMap.push_back({SCE_CTRL_RIGHT, KEY_DRIGHT});
    vitaToSwitchMap.push_back({SCE_CTRL_CROSS, KEY_A});
    vitaToSwitchMap.push_back({SCE_CTRL_CIRCLE, KEY_B});
    vitaToSwitchMap.push_back({SCE_CTRL_SQUARE, KEY_Y});
    vitaToSwitchMap.push_back({SCE_CTRL_TRIANGLE, KEY_X});
    vitaToSwitchMap.push_back({SCE_CTRL_L1, KEY_L});
    vitaToSwitchMap.push_back({SCE_CTRL_R1, KEY_R});
    vitaToSwitchMap.push_back({SCE_CTRL_START, KEY_PLUS});
    vitaToSwitchMap.push_back({SCE_CTRL_SELECT, KEY_MINUS});
    
    // Switch to PS Vita button mappings (reverse of above)
    for (const auto& pair : vitaToSwitchMap) {
        switchToVitaMap.push_back({pair.second, pair.first});
    }
}

uint32_t SwitchPlatform::mapVitaButtonToSwitch(uint32_t vitaButton) const {
    for (const auto& pair : vitaToSwitchMap) {
        if (pair.first == vitaButton) {
            return pair.second;
        }
    }
    return 0; // No mapping found
}

uint32_t SwitchPlatform::mapSwitchButtonToVita(uint32_t switchButton) const {
    for (const auto& pair : switchToVitaMap) {
        if (pair.first == switchButton) {
            return pair.second;
        }
    }
    return 0; // No mapping found
}

void SwitchPlatform::beginFrame() {
    // Update docked state
    docked = isDocked();
    
    // Update screen dimensions if docked state changed
    if (docked && screenWidth != 1920) {
        screenWidth = 1920;
        screenHeight = 1080;
    } else if (!docked && screenWidth != 1280) {
        screenWidth = 1280;
        screenHeight = 720;
    }
}

void SwitchPlatform::endFrame() {
    // Nothing to do here for now
}

void SwitchPlatform::setVsyncEnabled(bool enabled) {
    // Set vsync through SDL
    SDL_GL_SetSwapInterval(enabled ? 1 : 0);
}

void SwitchPlatform::setPerformanceProfile(int profile) {
    if (profile < 0 || profile > 2) {
        return; // Invalid profile
    }
    
    performanceProfile = profile;
    
    // Set CPU and GPU clock rates based on profile
    switch (profile) {
        case 0: // Power saving
            // Lower clock rates
            break;
        case 1: // Balanced (default)
            // Default clock rates
            break;
        case 2: // Performance
            // Higher clock rates
            break;
    }
}

int SwitchPlatform::getCurrentPerformanceProfile() const {
    return performanceProfile;
}
