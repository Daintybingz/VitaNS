#pragma once

#include "sceCtrl.h"
#include "sceTouch.h"
#include "../display/sceDisplay.h"
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>

namespace firmware {
namespace modules {
namespace input {

// Input manager class to coordinate between controller and touch inputs
class InputManager {
public:
    static InputManager& getInstance() {
        static InputManager instance;
        return instance;
    }

    // Initialize the input manager
    int initialize();

    // Terminate the input manager
    int terminate();

    // Update input state from Nintendo Switch
    void updateInputState(uint32_t buttons, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry);

    // Update touch state from Nintendo Switch
    void updateTouchState(int x, int y, bool pressed);

    // Toggle rear touch mode
    void toggleRearTouchMode();

    // Check if rear touch is active
    bool isRearTouchActive() const;

private:
    InputManager();
    ~InputManager();

    // Delete copy and move constructors and assign operators
    InputManager(const InputManager&) = delete;
    InputManager(InputManager&&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager& operator=(InputManager&&) = delete;

    // Rear touch overlay callback
    static void rearTouchOverlayCallback(void* userData, uint32_t frameBuffer, uint32_t pitch, uint32_t x, uint32_t y);

    // Input polling thread function
    void inputPollingThread();

    // Input initialized flag
    bool initialized;

    // Input polling thread
    std::unique_ptr<std::thread> polling_thread;

    // Thread running flag
    std::atomic<bool> thread_running;

    // Button combination for toggling rear touch mode
    uint32_t rear_touch_toggle_buttons;

    // Previous button state for detecting button combinations
    uint32_t previous_buttons;

    // Mutex for thread safety
    mutable std::mutex mutex;
};

} // namespace input
} // namespace modules
} // namespace firmware
