#pragma once

#include <switch.h>
#include <SDL2/SDL.h>
#include <string>
#include <vector>
#include <memory>

// Missing libnx constants and types (if not provided by switch.h)
#ifndef KEY_DUP
#define KEY_DUP          0x0000000000000008ULL
#define KEY_DDOWN        0x0000000000000002ULL
#define KEY_DLEFT        0x0000000000000004ULL
#define KEY_DRIGHT       0x0000000000000001ULL
#define KEY_A            0x0000000000000010ULL
#define KEY_B            0x0000000000000020ULL
#define KEY_X            0x0000000000000040ULL
#define KEY_Y            0x0000000000000080ULL
#define KEY_L            0x0000000000000100ULL
#define KEY_R            0x0000000000000200ULL
#define KEY_PLUS         0x0000000000000400ULL
#define KEY_MINUS        0x0000000000000800ULL
#endif

#ifndef CONTROLLER_P1_AUTO
#define CONTROLLER_P1_AUTO 0
#endif

// Touch position structure (if not defined by libnx)
#ifndef touchPosition
typedef struct {
    uint16_t px;
    uint16_t py;
} touchPosition;
#endif

// PS Vita control constants (not provided by libnx)
#define SCE_CTRL_UP          0x00000010
#define SCE_CTRL_DOWN        0x00000040
#define SCE_CTRL_LEFT        0x00000080
#define SCE_CTRL_RIGHT       0x00000020
#define SCE_CTRL_CROSS       0x00000001
#define SCE_CTRL_CIRCLE      0x00000002
#define SCE_CTRL_SQUARE      0x00000004
#define SCE_CTRL_TRIANGLE    0x00000008
#define SCE_CTRL_L1          0x00000100
#define SCE_CTRL_R1          0x00000200
#define SCE_CTRL_START       0x00000010
#define SCE_CTRL_SELECT      0x00000020

// Platform abstraction layer for Nintendo Switch
class SwitchPlatform {
public:
    // Singleton pattern
    static SwitchPlatform& getInstance();
    
    // Initialization and cleanup
    bool initialize();
    void finalize();
    
    // System information
    bool isDocked() const;
    int getScreenWidth() const;
    int getScreenHeight() const;
    uint64_t getAvailableMemory() const;
    std::string getSystemLanguage() const;
    
    // File system
    std::string getBasePath() const;
    std::string getSaveDataPath() const;
    std::string getGamePath() const;
    bool createDirectory(const std::string& path) const;
    
    // Input handling
    void updateInputState();
    bool isButtonPressed(uint32_t button) const;
    bool isButtonHeld(uint32_t button) const;
    bool isButtonReleased(uint32_t button) const;
    void getTouchPosition(int& x, int& y) const;
    bool isTouchActive() const;
    
    // Mapping PS Vita controls to Switch
    uint32_t mapVitaButtonToSwitch(uint32_t vitaButton) const;
    uint32_t mapSwitchButtonToVita(uint32_t switchButton) const;
    
    // Display
    void beginFrame();
    void endFrame();
    void setVsyncEnabled(bool enabled);
    
    // Performance
    void setPerformanceProfile(int profile); // 0=powersave, 1=balanced, 2=performance
    int getCurrentPerformanceProfile() const;
    
private:
    SwitchPlatform();  // Private constructor for singleton
    ~SwitchPlatform();
    
    // Prevent copying
    SwitchPlatform(const SwitchPlatform&) = delete;
    SwitchPlatform& operator=(const SwitchPlatform&) = delete;
    
    // Internal state
    bool initialized;
    bool docked;
    int screenWidth;
    int screenHeight;
    int performanceProfile;
    
    // Input state
    uint64_t currentButtonState;
    uint64_t previousButtonState;
    touchPosition touchPos;
    bool touchActive;
    
    // Button mapping tables
    std::vector<std::pair<uint32_t, uint32_t>> vitaToSwitchMap;
    std::vector<std::pair<uint32_t, uint32_t>> switchToVitaMap;
    
    // Initialize button mappings
    void initializeButtonMappings();
};
