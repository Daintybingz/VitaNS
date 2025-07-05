#include "switch_input.h"
#include "../modules/SceTouch/SceTouch.h"
#include <cstdio>
#include <chrono>

#include <switch.h>
#include <math.h>

SwitchInput::SwitchInput() : initialized(false), touchModule(nullptr) {
    // Initialize controller state to zero
    state.buttons = 0;
    state.left_stick_x = 0;
    state.left_stick_y = 0;
    state.right_stick_x = 0;
    state.right_stick_y = 0;
    
    // Initialize touch state
    touchState.touched = false;
    touchState.x = 0.0f;
    touchState.y = 0.0f;
    touchState.timestamp = 0;
}

SwitchInput::~SwitchInput() {
    finalize();
}

bool SwitchInput::initialize() {
    printf("[SwitchInput] Initializing input system\n");
    
    // On actual Switch hardware, we would initialize the controller library
    // padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    // padInitializeDefault(&pad);
    
    initialized = true;
    return true;
}

void SwitchInput::update() {
    if (!initialized) {
        return;
    }
    
    // On actual Switch hardware, we would read the controller state
    // padUpdate(&pad);
    // u64 kDown = padGetButtonsDown(&pad);
    // state.buttons = (uint32_t)kDown;
    // JoystickPosition stick_left, stick_right;
    // padGetStickPos(&pad, 0, &stick_left);
    // padGetStickPos(&pad, 1, &stick_right);
    // state.left_stick_x = stick_left.x;
    // state.left_stick_y = stick_left.y;
    // state.right_stick_x = stick_right.x;
    // state.right_stick_y = stick_right.y;
    
    // On actual Switch hardware, we would read the touch state
    // HidTouchScreenState touchState;
    // hidGetTouchScreenStates(&touchState, 1);
    // if (touchState.count > 0) {
    //     touchState.touched = true;
    //     touchState.x = touchState.touches[0].x;
    //     touchState.y = touchState.touches[0].y;
    //     auto now = std::chrono::steady_clock::now();
    //     auto duration = now.time_since_epoch();
    //     touchState.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    // } else {
    //     touchState.touched = false;
    // }
    
    // For now, we'll simulate some input for testing
    // In a real implementation, this would be replaced with actual controller input
    static int frame = 0;
    frame++;
    
    // Simulate touch input (circular motion) every 120 frames
    if (frame % 120 < 60) {
        float angle = (frame % 60) * (3.14159f * 2.0f / 60.0f);
        touchState.touched = true;
        touchState.x = 640.0f + cosf(angle) * 200.0f; // Center X + radius * cos(angle)
        touchState.y = 360.0f + sinf(angle) * 200.0f; // Center Y + radius * sin(angle)
        
        // Update timestamp
        auto now = std::chrono::steady_clock::now();
        auto duration = now.time_since_epoch();
        touchState.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        
        // Forward to SceTouch module if available
        if (touchModule) {
            touchModule->updateTouchState(touchState.touched, touchState.x, touchState.y);
        }
    } else {
        touchState.touched = false;
        
        // Forward to SceTouch module if available
        if (touchModule) {
            touchModule->updateTouchState(false, 0.0f, 0.0f);
        }
    }
    
    // Simulate pressing A button every 60 frames
    if (frame % 60 < 30) {
        state.buttons |= SWITCH_BUTTON_A;
    } else {
        state.buttons &= ~SWITCH_BUTTON_A;
    }
    
    // Simulate moving left stick in a circle
    float angle = (float)frame * 0.05f;
    state.left_stick_x = (int16_t)(cosf(angle) * 20000.0f);
    state.left_stick_y = (int16_t)(sinf(angle) * 20000.0f);
    
    // Keep right stick centered
    state.right_stick_x = 0;
    state.right_stick_y = 0;
}

void SwitchInput::finalize() {
    if (initialized) {
        printf("[SwitchInput] Finalizing input system\n");
        
        // On actual Switch hardware, we would clean up the controller library
        // No explicit cleanup needed for pad library
        
        initialized = false;
    }
}

const SwitchControllerState& SwitchInput::getControllerState() const {
    return state;
}

const SwitchTouchState& SwitchInput::getTouchState() const {
    return touchState;
}

void SwitchInput::setTouchModule(SceTouch* module) {
    touchModule = module;
}

bool SwitchInput::isButtonPressed(uint32_t button) const {
    return (state.buttons & button) != 0;
}

float SwitchInput::getLeftStickX() const {
    return state.left_stick_x / 32767.0f;
}

float SwitchInput::getLeftStickY() const {
    return state.left_stick_y / 32767.0f;
}

float SwitchInput::getRightStickX() const {
    return state.right_stick_x / 32767.0f;
}

float SwitchInput::getRightStickY() const {
    return state.right_stick_y / 32767.0f;
}

uint8_t SwitchInput::getLeftStickXByte() const {
    // Convert from -32768..32767 to 0..255 (with 128 as center)
    return (uint8_t)((getLeftStickX() + 1.0f) * 127.5f);
}

uint8_t SwitchInput::getLeftStickYByte() const {
    // Convert from -32768..32767 to 0..255 (with 128 as center)
    return (uint8_t)((getLeftStickY() + 1.0f) * 127.5f);
}

uint8_t SwitchInput::getRightStickXByte() const {
    // Convert from -32768..32767 to 0..255 (with 128 as center)
    return (uint8_t)((getRightStickX() + 1.0f) * 127.5f);
}

uint8_t SwitchInput::getRightStickYByte() const {
    // Convert from -32768..32767 to 0..255 (with 128 as center)
    return (uint8_t)((getRightStickY() + 1.0f) * 127.5f);
}
