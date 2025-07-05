#pragma once

#include <switch.h>
#include <map>
#include <string>
#include <vector>
#include "switch_input.h"

// PS Vita button definitions (from Vita3K)
#define SCE_CTRL_UP         0x00000010
#define SCE_CTRL_RIGHT      0x00000020
#define SCE_CTRL_DOWN       0x00000040
#define SCE_CTRL_LEFT       0x00000080
#define SCE_CTRL_TRIANGLE   0x00001000
#define SCE_CTRL_CIRCLE     0x00002000
#define SCE_CTRL_CROSS      0x00004000
#define SCE_CTRL_SQUARE     0x00008000
#define SCE_CTRL_SELECT     0x00000001
#define SCE_CTRL_START      0x00000008
#define SCE_CTRL_L1         0x00000100
#define SCE_CTRL_R1         0x00000200
#define SCE_CTRL_L2         0x00000400
#define SCE_CTRL_R2         0x00000800
#define SCE_CTRL_L3         0x00040000
#define SCE_CTRL_R3         0x00080000

// Touch panel dimensions for PS Vita
#define VITA_TOUCH_WIDTH    960
#define VITA_TOUCH_HEIGHT   544
#define VITA_BACK_TOUCH_WIDTH  960
#define VITA_BACK_TOUCH_HEIGHT 544

// Input mapping class for Nintendo Switch
class InputMapping {
public:
    // Singleton pattern
    static InputMapping& getInstance();
    
    // Initialize the input mapping system
    bool initialize();
    
    // Update input state
    void update();
    
    // Get mapped PS Vita button state
    uint32_t getButtonState() const;
    
    // Check if a specific PS Vita button is pressed
    bool isButtonPressed(uint32_t button) const;
    
    // Get front touch data (x, y, active)
    void getFrontTouchData(float& x, float& y, bool& active) const;
    
    // Get back touch data (x, y, active) - emulated on Switch
    void getBackTouchData(float& x, float& y, bool& active) const;
    
    // Get left analog stick data (normalized -1.0 to 1.0)
    void getLeftAnalog(float& x, float& y) const;
    
    // Get right analog stick data (normalized -1.0 to 1.0)
    void getRightAnalog(float& x, float& y) const;
    
    // Set button mapping
    void setButtonMapping(uint32_t vitaButton, uint64_t switchButton);
    
    // Reset button mappings to default
    void resetButtonMappings();
    
    // Enable/disable rear touch panel emulation
    void setRearTouchEmulationEnabled(bool enabled);
    
    // Set rear touch emulation mode (0: right stick, 1: touch screen corners)
    void setRearTouchEmulationMode(int mode);
    
    // Save/load mappings
    bool saveMappings(const std::string& filename) const;
    bool loadMappings(const std::string& filename);
    
    // Set the SwitchInput instance to use for input
    void setSwitchInput(SwitchInput* input) { switchInput = input; }
    
private:
    InputMapping();  // Private constructor for singleton
    ~InputMapping();
    
    // Prevent copying
    InputMapping(const InputMapping&) = delete;
    InputMapping& operator=(const InputMapping&) = delete;
    
    // Internal state
    bool initialized;
    
    // Button mappings (Vita button -> Switch button)
    std::map<uint32_t, uint64_t> buttonMappings;
    
    // Current input state
    uint32_t currentVitaButtons;
    float leftAnalogX, leftAnalogY;
    float rightAnalogX, rightAnalogY;
    
    // Touch state
    bool frontTouchActive;
    float frontTouchX, frontTouchY;
    bool backTouchActive;
    float backTouchX, backTouchY;
    
    // Rear touch emulation settings
    bool rearTouchEmulationEnabled;
    int rearTouchEmulationMode;
    
    SwitchInput* switchInput; // Pointer to SwitchInput instance
    
    // Initialize default button mappings
    void initializeDefaultMappings();
    
    // Convert Switch stick values to normalized values
    void normalizeStickValues(int16_t x, int16_t y, float& outX, float& outY) const;
    
    // Map Switch touch coordinates to PS Vita coordinates
    void mapTouchCoordinates(uint32_t x, uint32_t y, float& outX, float& outY) const;
    
    // Emulate rear touch panel using right stick
    void updateRearTouchEmulation();
};
