#include "sceCtrl.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <algorithm>

namespace firmware {
namespace modules {
namespace input {

// Global controller manager instance
static SceCtrlManager g_ctrl_manager;

// SceCtrlManager implementation
SceCtrlManager::SceCtrlManager()
    : initialized(false) {
}

SceCtrlManager::~SceCtrlManager() {
    if (initialized) {
        terminate();
    }
}

int SceCtrlManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (initialized) {
        std::cerr << "Controller manager is already initialized" << std::endl;
        return SCE_CTRL_ERROR_ALREADY_INIT;
    }
    
    std::cout << "Initializing controller manager" << std::endl;
    
    // Initialize controller states
    controllers.clear();
    
    // Initialize main controller (port 0)
    ControllerState state;
    state.buttons = 0;
    state.lx = 128;
    state.ly = 128;
    state.rx = 128;
    state.ry = 128;
    state.mode = SCE_CTRL_MODE_DIGITAL;
    state.sampling_rate = 60; // 60 Hz
    state.type = SCE_CTRL_TYPE_PHY;
    state.status = 1; // Connected
    state.battery_level = 5; // Full battery
    state.buffer.clear();
    controllers[0] = state;
    
    // Initialize button mappings
    button_mappings.clear();
    
    // Default mapping for Nintendo Switch to PS Vita
    std::map<int, int> default_mapping;
    // Nintendo Switch buttons to PS Vita buttons
    // This is a simplified mapping, in a real implementation this would be more complex
    default_mapping[0x00000001] = SCE_CTRL_SELECT;    // - button -> Select
    default_mapping[0x00000002] = SCE_CTRL_START;     // + button -> Start
    default_mapping[0x00000004] = SCE_CTRL_L3;        // L stick click -> L3
    default_mapping[0x00000008] = SCE_CTRL_R3;        // R stick click -> R3
    default_mapping[0x00000010] = SCE_CTRL_UP;        // Up -> Up
    default_mapping[0x00000020] = SCE_CTRL_RIGHT;     // Right -> Right
    default_mapping[0x00000040] = SCE_CTRL_DOWN;      // Down -> Down
    default_mapping[0x00000080] = SCE_CTRL_LEFT;      // Left -> Left
    default_mapping[0x00000100] = SCE_CTRL_LTRIGGER;  // L -> L trigger
    default_mapping[0x00000200] = SCE_CTRL_RTRIGGER;  // R -> R trigger
    default_mapping[0x00000400] = SCE_CTRL_L1;        // ZL -> L1
    default_mapping[0x00000800] = SCE_CTRL_R1;        // ZR -> R1
    default_mapping[0x00001000] = SCE_CTRL_TRIANGLE;  // X -> Triangle
    default_mapping[0x00002000] = SCE_CTRL_CIRCLE;    // A -> Circle
    default_mapping[0x00004000] = SCE_CTRL_CROSS;     // B -> Cross
    default_mapping[0x00008000] = SCE_CTRL_SQUARE;    // Y -> Square
    
    button_mappings[0] = default_mapping;
    
    initialized = true;
    std::cout << "Controller manager initialized successfully" << std::endl;
    return 0;
}

int SceCtrlManager::terminate() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    std::cout << "Terminating controller manager" << std::endl;
    
    // Clear controller states
    controllers.clear();
    
    // Clear button mappings
    button_mappings.clear();
    
    initialized = false;
    std::cout << "Controller manager terminated successfully" << std::endl;
    return 0;
}

int SceCtrlManager::set_mode(int port, int mode) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    if (port < 0 || port > 1) {
        std::cerr << "Invalid controller port: " << port << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (mode != SCE_CTRL_MODE_DIGITAL && mode != SCE_CTRL_MODE_ANALOG && mode != SCE_CTRL_MODE_ANALOG_WIDE) {
        std::cerr << "Invalid controller mode: " << mode << std::endl;
        return SCE_CTRL_ERROR_INVALID_MODE;
    }
    
    auto it = controllers.find(port);
    if (it == controllers.end()) {
        std::cerr << "Controller not found for port: " << port << std::endl;
        return SCE_CTRL_ERROR_NO_DEVICE;
    }
    
    it->second.mode = mode;
    std::cout << "Set controller mode for port " << port << " to " << mode << std::endl;
    return 0;
}

int SceCtrlManager::get_mode(int port, int *mode) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    if (port < 0 || port > 1) {
        std::cerr << "Invalid controller port: " << port << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (!mode) {
        std::cerr << "Invalid mode pointer" << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    auto it = controllers.find(port);
    if (it == controllers.end()) {
        std::cerr << "Controller not found for port: " << port << std::endl;
        return SCE_CTRL_ERROR_NO_DEVICE;
    }
    
    *mode = it->second.mode;
    return 0;
}

int SceCtrlManager::set_sampling_rate(int port, int rate) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    if (port < 0 || port > 1) {
        std::cerr << "Invalid controller port: " << port << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (rate != 60 && rate != 120 && rate != 240) {
        std::cerr << "Invalid sampling rate: " << rate << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    auto it = controllers.find(port);
    if (it == controllers.end()) {
        std::cerr << "Controller not found for port: " << port << std::endl;
        return SCE_CTRL_ERROR_NO_DEVICE;
    }
    
    it->second.sampling_rate = rate;
    std::cout << "Set controller sampling rate for port " << port << " to " << rate << " Hz" << std::endl;
    return 0;
}

int SceCtrlManager::get_sampling_rate(int port, int *rate) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    if (port < 0 || port > 1) {
        std::cerr << "Invalid controller port: " << port << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (!rate) {
        std::cerr << "Invalid rate pointer" << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    auto it = controllers.find(port);
    if (it == controllers.end()) {
        std::cerr << "Controller not found for port: " << port << std::endl;
        return SCE_CTRL_ERROR_NO_DEVICE;
    }
    
    *rate = it->second.sampling_rate;
    return 0;
}

int SceCtrlManager::set_button_mapping(int port, int buttons, int mapped_buttons) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    if (port < 0 || port > 1) {
        std::cerr << "Invalid controller port: " << port << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    auto it = button_mappings.find(port);
    if (it == button_mappings.end()) {
        button_mappings[port] = std::map<int, int>();
        it = button_mappings.find(port);
    }
    
    it->second[buttons] = mapped_buttons;
    std::cout << "Set button mapping for port " << port << ": " << std::hex << buttons << " -> " << mapped_buttons << std::dec << std::endl;
    return 0;
}

int SceCtrlManager::read_data(int port, SceCtrlData *data, int count) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    if (port < 0 || port > 1) {
        std::cerr << "Invalid controller port: " << port << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (!data) {
        std::cerr << "Invalid data pointer" << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (count <= 0) {
        std::cerr << "Invalid count: " << count << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    auto it = controllers.find(port);
    if (it == controllers.end()) {
        std::cerr << "Controller not found for port: " << port << std::endl;
        return SCE_CTRL_ERROR_NO_DEVICE;
    }
    
    const ControllerState &state = it->second;
    
    // Fill data
    for (int i = 0; i < count; i++) {
        data[i].timeStamp = get_timestamp();
        data[i].buttons = map_buttons(port, state.buttons);
        data[i].lx = state.lx;
        data[i].ly = state.ly;
        data[i].rx = state.rx;
        data[i].ry = state.ry;
        memset(data[i].reserved, 0, sizeof(data[i].reserved));
    }
    
    return count;
}

int SceCtrlManager::peek_data(int port, SceCtrlData *data, int count) {
    // Same as read_data, but doesn't remove data from the buffer
    return read_data(port, data, count);
}

int SceCtrlManager::read_buffer_data(int port, SceCtrlData *data, int count) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    if (port < 0 || port > 1) {
        std::cerr << "Invalid controller port: " << port << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (!data) {
        std::cerr << "Invalid data pointer" << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (count <= 0) {
        std::cerr << "Invalid count: " << count << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    auto it = controllers.find(port);
    if (it == controllers.end()) {
        std::cerr << "Controller not found for port: " << port << std::endl;
        return SCE_CTRL_ERROR_NO_DEVICE;
    }
    
    ControllerState &state = it->second;
    
    // Copy data from buffer
    int copied = 0;
    while (copied < count && !state.buffer.empty()) {
        data[copied] = state.buffer.front();
        state.buffer.erase(state.buffer.begin());
        copied++;
    }
    
    // If buffer is empty, fill with current state
    if (copied == 0) {
        data[0].timeStamp = get_timestamp();
        data[0].buttons = map_buttons(port, state.buttons);
        data[0].lx = state.lx;
        data[0].ly = state.ly;
        data[0].rx = state.rx;
        data[0].ry = state.ry;
        memset(data[0].reserved, 0, sizeof(data[0].reserved));
        copied = 1;
    }
    
    return copied;
}

int SceCtrlManager::set_actuator(int port, int small, int large) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    if (port < 0 || port > 1) {
        std::cerr << "Invalid controller port: " << port << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (small < 0 || small > 255 || large < 0 || large > 255) {
        std::cerr << "Invalid actuator values: small=" << small << ", large=" << large << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    auto it = controllers.find(port);
    if (it == controllers.end()) {
        std::cerr << "Controller not found for port: " << port << std::endl;
        return SCE_CTRL_ERROR_NO_DEVICE;
    }
    
    // In a real implementation, this would send vibration commands to the controller
    std::cout << "Set controller actuator for port " << port << ": small=" << small << ", large=" << large << std::endl;
    return 0;
}

int SceCtrlManager::get_controller_info(int port, int *type, int *status) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    if (port < 0 || port > 1) {
        std::cerr << "Invalid controller port: " << port << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (!type || !status) {
        std::cerr << "Invalid type or status pointer" << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    auto it = controllers.find(port);
    if (it == controllers.end()) {
        std::cerr << "Controller not found for port: " << port << std::endl;
        return SCE_CTRL_ERROR_NO_DEVICE;
    }
    
    *type = it->second.type;
    *status = it->second.status;
    return 0;
}

int SceCtrlManager::get_battery_level(int port, int *level) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Controller manager is not initialized" << std::endl;
        return SCE_CTRL_ERROR_NOT_INIT;
    }
    
    if (port < 0 || port > 1) {
        std::cerr << "Invalid controller port: " << port << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    if (!level) {
        std::cerr << "Invalid level pointer" << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    auto it = controllers.find(port);
    if (it == controllers.end()) {
        std::cerr << "Controller not found for port: " << port << std::endl;
        return SCE_CTRL_ERROR_NO_DEVICE;
    }
    
    *level = it->second.battery_level;
    return 0;
}

void SceCtrlManager::update_state(uint32_t buttons, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return;
    }
    
    // Update main controller state
    auto it = controllers.find(0);
    if (it != controllers.end()) {
        ControllerState &state = it->second;
        state.buttons = buttons;
        state.lx = lx;
        state.ly = ly;
        state.rx = rx;
        state.ry = ry;
        
        // Add to buffer
        SceCtrlData data;
        data.timeStamp = get_timestamp();
        data.buttons = map_buttons(0, buttons);
        data.lx = lx;
        data.ly = ly;
        data.rx = rx;
        data.ry = ry;
        memset(data.reserved, 0, sizeof(data.reserved));
        
        state.buffer.push_back(data);
        
        // Limit buffer size
        if (state.buffer.size() > 64) {
            state.buffer.erase(state.buffer.begin());
        }
    }
}

uint64_t SceCtrlManager::get_timestamp() const {
    // Get current time in microseconds
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

uint32_t SceCtrlManager::map_buttons(int port, uint32_t buttons) const {
    uint32_t mapped_buttons = 0;
    
    auto port_it = button_mappings.find(port);
    if (port_it != button_mappings.end()) {
        const auto &mapping = port_it->second;
        
        // Apply button mapping
        for (const auto &pair : mapping) {
            if (buttons & pair.first) {
                mapped_buttons |= pair.second;
            }
        }
    } else {
        // No mapping, use buttons as is
        mapped_buttons = buttons;
    }
    
    return mapped_buttons;
}

// Module function implementations
int sceCtrlInit() {
    return g_ctrl_manager.initialize();
}

int sceCtrlExit() {
    return g_ctrl_manager.terminate();
}

int sceCtrlSetSamplingMode(int mode) {
    return g_ctrl_manager.set_mode(0, mode);
}

int sceCtrlGetSamplingMode(int *mode) {
    return g_ctrl_manager.get_mode(0, mode);
}

int sceCtrlSetSamplingModeExt(int port, int mode) {
    return g_ctrl_manager.set_mode(port, mode);
}

int sceCtrlGetSamplingModeExt(int port, int *mode) {
    return g_ctrl_manager.get_mode(port, mode);
}

int sceCtrlReadBufferPositive(int port, SceCtrlData *data, int count) {
    return g_ctrl_manager.read_data(port, data, count);
}

int sceCtrlReadBufferNegative(int port, SceCtrlData *data, int count) {
    // Same as positive, but with inverted buttons (not implemented)
    return g_ctrl_manager.read_data(port, data, count);
}

int sceCtrlPeekBufferPositive(int port, SceCtrlData *data, int count) {
    return g_ctrl_manager.peek_data(port, data, count);
}

int sceCtrlPeekBufferNegative(int port, SceCtrlData *data, int count) {
    // Same as positive, but with inverted buttons (not implemented)
    return g_ctrl_manager.peek_data(port, data, count);
}

int sceCtrlReadBuffer(int port, SceCtrlData *data, int count) {
    return g_ctrl_manager.read_buffer_data(port, data, count);
}

int sceCtrlPeekBuffer(int port, SceCtrlData *data, int count) {
    // Same as read, but doesn't remove data from the buffer
    return g_ctrl_manager.read_buffer_data(port, data, count);
}

int sceCtrlSetActuator(int port, int small, int large) {
    return g_ctrl_manager.set_actuator(port, small, large);
}

int sceCtrlGetControllerPortInfo(int *info) {
    if (!info) {
        std::cerr << "Invalid info pointer" << std::endl;
        return SCE_CTRL_ERROR_INVALID_ARG;
    }
    
    // Port 0: Physical controller
    info[0] = SCE_CTRL_TYPE_PHY;
    
    // Port 1: No controller
    info[1] = SCE_CTRL_TYPE_UNPAIRED;
    
    return 0;
}

int sceCtrlGetBatteryInfo(int port, int *level) {
    return g_ctrl_manager.get_battery_level(port, level);
}

int sceCtrlSetButtonMapping(int port, int buttons, int mapped_buttons) {
    return g_ctrl_manager.set_button_mapping(port, buttons, mapped_buttons);
}

int sceCtrlGetButtonMapping(int port, int buttons, int *mapped_buttons) {
    // Not implemented
    return SCE_CTRL_ERROR_NOT_SUPPORTED;
}

int sceCtrlSetRapidFire(int port, int mask, int trigger, int delay, int make, int break_time) {
    // Not implemented
    return SCE_CTRL_ERROR_NOT_SUPPORTED;
}

int sceCtrlGetRapidFire(int port, int mask, int *trigger, int *delay, int *make, int *break_time) {
    // Not implemented
    return SCE_CTRL_ERROR_NOT_SUPPORTED;
}

} // namespace input
} // namespace modules
} // namespace firmware
