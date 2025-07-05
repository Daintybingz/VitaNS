#pragma once
#include <cstdint>

// Nintendo Switch button definitions
#define SWITCH_BUTTON_A        (1 << 0)
#define SWITCH_BUTTON_B        (1 << 1)
#define SWITCH_BUTTON_X        (1 << 2)
#define SWITCH_BUTTON_Y        (1 << 3)
#define SWITCH_BUTTON_LSTICK   (1 << 4)
#define SWITCH_BUTTON_RSTICK   (1 << 5)
#define SWITCH_BUTTON_L        (1 << 6)
#define SWITCH_BUTTON_R        (1 << 7)
#define SWITCH_BUTTON_ZL       (1 << 8)
#define SWITCH_BUTTON_ZR       (1 << 9)
#define SWITCH_BUTTON_PLUS     (1 << 10)
#define SWITCH_BUTTON_MINUS    (1 << 11)
#define SWITCH_BUTTON_LEFT     (1 << 12)
#define SWITCH_BUTTON_UP       (1 << 13)
#define SWITCH_BUTTON_RIGHT    (1 << 14)
#define SWITCH_BUTTON_DOWN     (1 << 15)

// Switch controller state
struct SwitchControllerState {
    uint32_t buttons;
    int16_t left_stick_x;   // -32768 to 32767
    int16_t left_stick_y;   // -32768 to 32767
    int16_t right_stick_x;  // -32768 to 32767
    int16_t right_stick_y;  // -32768 to 32767
};

// Switch touch state
struct SwitchTouchState {
    bool touched;           // Is screen being touched
    float x;               // X position (0.0 to screen width)
    float y;               // Y position (0.0 to screen height)
    uint64_t timestamp;    // Timestamp of touch event
};

// Forward declaration
class SceTouch;

class SwitchInput {
public:
    SwitchInput();
    ~SwitchInput();

    // Initialize the input system
    bool initialize();
    
    // Update controller state (should be called every frame)
    void update();
    
    // Finalize the input system
    void finalize();
    
    // Set the SceTouch module for touch input forwarding
    void setTouchModule(SceTouch* touchModule);
    
    // Get current controller state
    const SwitchControllerState& getControllerState() const;
    
    // Get current touch state
    const SwitchTouchState& getTouchState() const;
    
    // Check if a button is pressed
    bool isButtonPressed(uint32_t button) const;
    
    // Get stick values (normalized from -1.0 to 1.0)
    float getLeftStickX() const;
    float getLeftStickY() const;
    float getRightStickX() const;
    float getRightStickY() const;
    
    // Convert to 8-bit unsigned value (0-255) for PS Vita compatibility
    uint8_t getLeftStickXByte() const;
    uint8_t getLeftStickYByte() const;
    uint8_t getRightStickXByte() const;
    uint8_t getRightStickYByte() const;

private:
    SwitchControllerState state;
    SwitchTouchState touchState;
    SceTouch* touchModule;
    bool initialized;
};
