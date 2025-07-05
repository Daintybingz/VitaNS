#include "sceTouch.h"
#include "sceCtrl.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <algorithm>

namespace firmware {
namespace modules {
namespace input {

// Global touch manager instance
static SceTouchManager g_touch_manager;

// SceTouchManager implementation
SceTouchManager::SceTouchManager()
    : initialized(false), rear_touch_mode(false), rear_touch_trigger_buttons(SCE_CTRL_LTRIGGER) {
}

SceTouchManager::~SceTouchManager() {
    if (initialized) {
        terminate();
    }
}

int SceTouchManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (initialized) {
        std::cerr << "Touch manager is already initialized" << std::endl;
        return SCE_TOUCH_ERROR_FATAL;
    }
    
    std::cout << "Initializing touch manager" << std::endl;
    
    // Initialize touch states
    touch_states.clear();
    
    // Initialize front touch panel
    TouchState front_state;
    front_state.mode = SCE_TOUCH_MODE_NORMAL;
    front_state.sampling_state = SCE_TOUCH_SAMPLING_STATE_START;
    memset(&front_state.data, 0, sizeof(SceTouchData));
    front_state.data.status = 0;
    front_state.data.reportNum = 0;
    
    // Set front panel info (PS Vita front panel is 1920x1088)
    front_state.panel_info.minAaX = 0;
    front_state.panel_info.minAaY = 0;
    front_state.panel_info.maxAaX = 1919;
    front_state.panel_info.maxAaY = 1087;
    front_state.panel_info.minDispX = 0;
    front_state.panel_info.minDispY = 0;
    front_state.panel_info.maxDispX = 959;
    front_state.panel_info.maxDispY = 543;
    front_state.panel_info.minForce = 1;
    front_state.panel_info.maxForce = 128;
    
    touch_states[SCE_TOUCH_PORT_FRONT] = front_state;
    
    // Initialize back touch panel
    TouchState back_state;
    back_state.mode = SCE_TOUCH_MODE_NORMAL;
    back_state.sampling_state = SCE_TOUCH_SAMPLING_STATE_START;
    memset(&back_state.data, 0, sizeof(SceTouchData));
    back_state.data.status = 0;
    back_state.data.reportNum = 0;
    
    // Set back panel info (PS Vita back panel is 1920x896)
    back_state.panel_info.minAaX = 0;
    back_state.panel_info.minAaY = 0;
    back_state.panel_info.maxAaX = 1919;
    back_state.panel_info.maxAaY = 895;
    back_state.panel_info.minDispX = 0;
    back_state.panel_info.minDispY = 0;
    back_state.panel_info.maxDispX = 959;
    back_state.panel_info.maxDispY = 447;
    back_state.panel_info.minForce = 1;
    back_state.panel_info.maxForce = 128;
    
    touch_states[SCE_TOUCH_PORT_BACK] = back_state;
    
    // Initialize rear touch mode
    rear_touch_mode = false;
    
    initialized = true;
    std::cout << "Touch manager initialized successfully" << std::endl;
    return 0;
}

int SceTouchManager::terminate() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Touch manager is not initialized" << std::endl;
        return SCE_TOUCH_ERROR_FATAL;
    }
    
    std::cout << "Terminating touch manager" << std::endl;
    
    // Clear touch states
    touch_states.clear();
    
    initialized = false;
    std::cout << "Touch manager terminated successfully" << std::endl;
    return 0;
}

int SceTouchManager::set_mode(int port, SceTouchMode mode) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Touch manager is not initialized" << std::endl;
        return SCE_TOUCH_ERROR_FATAL;
    }
    
    if (port < 0 || port >= SCE_TOUCH_PORT_MAX) {
        std::cerr << "Invalid touch port: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    auto it = touch_states.find(port);
    if (it == touch_states.end()) {
        std::cerr << "Touch port not found: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    it->second.mode = mode;
    std::cout << "Set touch mode for port " << port << " to " << mode << std::endl;
    return 0;
}

int SceTouchManager::get_mode(int port, SceTouchMode *mode) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Touch manager is not initialized" << std::endl;
        return SCE_TOUCH_ERROR_FATAL;
    }
    
    if (port < 0 || port >= SCE_TOUCH_PORT_MAX) {
        std::cerr << "Invalid touch port: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    if (!mode) {
        std::cerr << "Invalid mode pointer" << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    auto it = touch_states.find(port);
    if (it == touch_states.end()) {
        std::cerr << "Touch port not found: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    *mode = it->second.mode;
    return 0;
}

int SceTouchManager::set_sampling_state(int port, int state) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Touch manager is not initialized" << std::endl;
        return SCE_TOUCH_ERROR_FATAL;
    }
    
    if (port < 0 || port >= SCE_TOUCH_PORT_MAX) {
        std::cerr << "Invalid touch port: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    if (state != SCE_TOUCH_SAMPLING_STATE_STOP && state != SCE_TOUCH_SAMPLING_STATE_START) {
        std::cerr << "Invalid sampling state: " << state << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    auto it = touch_states.find(port);
    if (it == touch_states.end()) {
        std::cerr << "Touch port not found: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    it->second.sampling_state = state;
    std::cout << "Set touch sampling state for port " << port << " to " << state << std::endl;
    return 0;
}

int SceTouchManager::get_sampling_state(int port, int *state) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Touch manager is not initialized" << std::endl;
        return SCE_TOUCH_ERROR_FATAL;
    }
    
    if (port < 0 || port >= SCE_TOUCH_PORT_MAX) {
        std::cerr << "Invalid touch port: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    if (!state) {
        std::cerr << "Invalid state pointer" << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    auto it = touch_states.find(port);
    if (it == touch_states.end()) {
        std::cerr << "Touch port not found: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    *state = it->second.sampling_state;
    return 0;
}

int SceTouchManager::read_data(int port, SceTouchData *data) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Touch manager is not initialized" << std::endl;
        return SCE_TOUCH_ERROR_FATAL;
    }
    
    if (port < 0 || port >= SCE_TOUCH_PORT_MAX) {
        std::cerr << "Invalid touch port: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    if (!data) {
        std::cerr << "Invalid data pointer" << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    auto it = touch_states.find(port);
    if (it == touch_states.end()) {
        std::cerr << "Touch port not found: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    // Check if sampling is enabled
    if (it->second.sampling_state != SCE_TOUCH_SAMPLING_STATE_START) {
        memset(data, 0, sizeof(SceTouchData));
        data->timeStamp = get_timestamp();
        return 0;
    }
    
    // Copy touch data
    memcpy(data, &it->second.data, sizeof(SceTouchData));
    
    // Clear touch data after reading
    it->second.data.reportNum = 0;
    
    return 0;
}

int SceTouchManager::peek_data(int port, SceTouchData *data) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Touch manager is not initialized" << std::endl;
        return SCE_TOUCH_ERROR_FATAL;
    }
    
    if (port < 0 || port >= SCE_TOUCH_PORT_MAX) {
        std::cerr << "Invalid touch port: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    if (!data) {
        std::cerr << "Invalid data pointer" << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    auto it = touch_states.find(port);
    if (it == touch_states.end()) {
        std::cerr << "Touch port not found: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    // Check if sampling is enabled
    if (it->second.sampling_state != SCE_TOUCH_SAMPLING_STATE_START) {
        memset(data, 0, sizeof(SceTouchData));
        data->timeStamp = get_timestamp();
        return 0;
    }
    
    // Copy touch data without clearing
    memcpy(data, &it->second.data, sizeof(SceTouchData));
    
    return 0;
}

int SceTouchManager::get_panel_info(int port, SceTouchPanelInfo *info) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Touch manager is not initialized" << std::endl;
        return SCE_TOUCH_ERROR_FATAL;
    }
    
    if (port < 0 || port >= SCE_TOUCH_PORT_MAX) {
        std::cerr << "Invalid touch port: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    if (!info) {
        std::cerr << "Invalid info pointer" << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    auto it = touch_states.find(port);
    if (it == touch_states.end()) {
        std::cerr << "Touch port not found: " << port << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    // Copy panel info
    memcpy(info, &it->second.panel_info, sizeof(SceTouchPanelInfo));
    
    return 0;
}

void SceTouchManager::update_state(int x, int y, bool pressed, uint32_t controller_buttons) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return;
    }
    
    // Determine which touch panel to update
    int port;
    
    // Check if we're in rear touch mode or if rear touch trigger buttons are pressed
    bool use_rear_touch = rear_touch_mode || should_activate_rear_touch(controller_buttons);
    
    if (use_rear_touch) {
        port = SCE_TOUCH_PORT_BACK;
    } else {
        port = SCE_TOUCH_PORT_FRONT;
    }
    
    auto it = touch_states.find(port);
    if (it == touch_states.end()) {
        return;
    }
    
    // Check if sampling is enabled
    if (it->second.sampling_state != SCE_TOUCH_SAMPLING_STATE_START) {
        return;
    }
    
    // Update touch data
    SceTouchData &data = it->second.data;
    data.timeStamp = get_timestamp();
    
    if (pressed) {
        // Convert Nintendo Switch touch coordinates to PS Vita coordinates
        // Switch touchscreen is 1280x720, we need to map to Vita's resolution
        
        // For front panel (1920x1088)
        uint16_t vita_x, vita_y;
        
        if (port == SCE_TOUCH_PORT_FRONT) {
            vita_x = static_cast<uint16_t>((x * 1920) / 1280);
            vita_y = static_cast<uint16_t>((y * 1088) / 720);
        } else {
            // For back panel (1920x896)
            vita_x = static_cast<uint16_t>((x * 1920) / 1280);
            vita_y = static_cast<uint16_t>((y * 896) / 720);
        }
        
        // Add touch report
        if (data.reportNum < SCE_TOUCH_MAX_REPORT) {
            data.report[data.reportNum].id = 0; // First finger
            data.report[data.reportNum].x = vita_x;
            data.report[data.reportNum].y = vita_y;
            data.report[data.reportNum].z = 128; // Medium pressure
            data.reportNum++;
        }
    } else {
        // Clear touch data when not pressed
        data.reportNum = 0;
    }
    
    // Update status
    data.status = data.reportNum > 0 ? 1 : 0;
    
    // If we're using the rear touch panel, provide visual feedback
    if (use_rear_touch && pressed) {
        std::cout << "Rear touch panel active at (" << x << ", " << y << ")" << std::endl;
    }
}

void SceTouchManager::toggle_rear_touch_mode() {
    std::lock_guard<std::mutex> lock(mutex);
    
    rear_touch_mode = !rear_touch_mode;
    std::cout << "Rear touch mode " << (rear_touch_mode ? "enabled" : "disabled") << std::endl;
}

bool SceTouchManager::is_rear_touch_active() const {
    std::lock_guard<std::mutex> lock(mutex);
    
    return rear_touch_mode;
}

uint64_t SceTouchManager::get_timestamp() const {
    // Get current time in microseconds
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

bool SceTouchManager::should_activate_rear_touch(uint32_t controller_buttons) const {
    // Check if the rear touch trigger buttons are pressed
    return (controller_buttons & rear_touch_trigger_buttons) != 0;
}

// Module function implementations
int sceTouchInit() {
    return g_touch_manager.initialize();
}

int sceTouchExit() {
    return g_touch_manager.terminate();
}

int sceTouchSetMode(int port, SceTouchMode mode) {
    return g_touch_manager.set_mode(port, mode);
}

int sceTouchGetMode(int port, SceTouchMode *mode) {
    return g_touch_manager.get_mode(port, mode);
}

int sceTouchSetSamplingState(int port, int state) {
    return g_touch_manager.set_sampling_state(port, state);
}

int sceTouchGetSamplingState(int port, int *state) {
    return g_touch_manager.get_sampling_state(port, state);
}

int sceTouchRead(int port, SceTouchData *data, int count) {
    if (count <= 0) {
        std::cerr << "Invalid count: " << count << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    // We only support reading one frame at a time
    return g_touch_manager.read_data(port, data);
}

int sceTouchPeek(int port, SceTouchData *data, int count) {
    if (count <= 0) {
        std::cerr << "Invalid count: " << count << std::endl;
        return SCE_TOUCH_ERROR_INVALID_ARG;
    }
    
    // We only support peeking one frame at a time
    return g_touch_manager.peek_data(port, data);
}

int sceTouchGetPanelInfo(int port, SceTouchPanelInfo *info) {
    return g_touch_manager.get_panel_info(port, info);
}

int sceTouchEnableTouchForce(int port) {
    // Not implemented
    return 0;
}

int sceTouchDisableTouchForce(int port) {
    // Not implemented
    return 0;
}

int sceTouchGetProcessInfo(int pid, int *process_info) {
    // Not implemented
    return 0;
}

// Custom function to toggle rear touch mode
void toggleRearTouchMode() {
    g_touch_manager.toggle_rear_touch_mode();
}

// Custom function to check if rear touch is active
bool isRearTouchActive() {
    return g_touch_manager.is_rear_touch_active();
}

} // namespace input
} // namespace modules
} // namespace firmware
