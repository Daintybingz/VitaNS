# VitaNS Technical Design Document

## Architecture Overview

VitaNS adapts the Vita3K architecture to run on Nintendo Switch hardware. This document outlines the technical design decisions and implementation details for the port.

## System Components

### 1. Core Emulation Layer

The core emulation layer consists of several key components that need to be adapted for the Switch:

#### 1.1 CPU Emulation

**Original Implementation:**
- Vita3K uses Unicorn and Dynarmic CPU backends for ARM emulation
- PS Vita uses ARM Cortex-A9 CPU

**Switch Adaptation:**
- Optimize for ARM-to-ARM translation (Switch uses ARM Cortex-A57/A53)
- Potential for direct code execution in some cases
- Memory mapping adjustments for Switch's memory layout

**Implementation Plan:**
```cpp
// Switch-specific CPU implementation
class SwitchCPUBackend : public CPUInterface {
public:
    // Implement CPU interface methods optimized for Switch
    virtual int run() override;
    virtual void stop() override;
    // ...
};
```

#### 1.2 Memory Management

**Original Implementation:**
- PS Vita has 512MB RAM with specific memory layout
- Vita3K virtualizes this memory space

**Switch Adaptation:**
- Utilize Switch's 4GB RAM (3.2GB available)
- Maintain PS Vita memory layout within virtual address space
- Optimize memory allocation for Switch

**Implementation Plan:**
```cpp
// Switch-specific memory management
void init_memory_for_switch() {
    // Allocate memory for PS Vita address space
    // Map memory regions according to PS Vita layout
    // Set up memory protection
}
```

### 2. Graphics Subsystem

#### 2.1 Renderer

**Original Implementation:**
- Vita3K supports OpenGL and Vulkan renderers
- PS Vita GPU is PowerVR SGX543MP4+
- SceDisplay module handles frame buffer management

**Switch Adaptation:**
- Use OpenGL ES 3.2 for Switch
- Adapt shaders for compatibility
- Implement resolution scaling for docked/handheld modes
- Integrate display system with input handling for visual feedback

**Implementation Details:**
```cpp
// Switch-specific renderer with input integration
class SceDisplayManager {
public:
    // Initialize display system
    int initialize();
    
    // Set frame buffer with overlay support
    int set_frame_buf(const SceDisplayFrameBuf *pFrameBuf, int updateTiming);
    
    // Register callback for rear touch panel overlay
    void register_rear_touch_overlay_callback(void (*callback)(void*, uint32_t, uint32_t, uint32_t, uint32_t), void* userData);
    
    // Show/hide rear touch overlay
    void show_rear_touch_overlay(bool show);
    
    // Update rear touch position for visual feedback
    void update_rear_touch_position(int x, int y);
};
```

#### 2.2 Shader Translation

**Original Implementation:**
- Vita3K translates GXP shaders to GLSL/SPIR-V

**Switch Adaptation:**
- Adapt shader translation for GLSL ES
- Optimize shader performance for Tegra X1

**Implementation Plan:**
```cpp
// Shader adaptation for Switch
std::string translate_shader_for_switch(const SceGxmProgram *program) {
    // Translate GXP shader to GLSL ES compatible with Switch
    // Apply Switch-specific optimizations
    return glsl_es_code;
}
```

### 3. Input System

**Original Implementation:**
- PS Vita has front/rear touch, buttons, analog sticks
- SceCtrl module handles controller input
- SceTouch module handles touch input for both panels

**Switch Adaptation:**
- Map Joy-Con buttons to PS Vita buttons with customizable mappings
- Use touch screen for front touch panel emulation
- Implement hybrid approach for rear touch panel emulation:
  1. Hold L trigger while touching to simulate rear touch
  2. Toggle mode with L+R+ZL+ZR to switch touchscreen between front/rear panels
- Provide visual feedback for rear touch panel interactions
- Integrate with display system for consistent user experience

**Implementation Details:**
```cpp
// Comprehensive input management system
class InputManager {
public:
    // Initialize all input subsystems
    int initialize();
    
    // Update controller state from Nintendo Switch inputs
    void updateInputState(uint32_t buttons, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry);
    
    // Update touch state with hybrid rear touch handling
    void updateTouchState(int x, int y, bool pressed);
    
    // Toggle between front/rear touch modes
    void toggleRearTouchMode();
    
    // Visual feedback for rear touch panel
    static void rearTouchOverlayCallback(void* userData, uint32_t frameBuffer, 
                                        uint32_t pitch, uint32_t x, uint32_t y);
private:
    // Input polling thread for continuous updates
    void inputPollingThread();
    
    // Button combination for toggling rear touch mode
    uint32_t rear_touch_toggle_buttons;
};
```

**SceCtrl Implementation:**
```cpp
class SceCtrlManager {
public:
    // Initialize controller system
    int initialize();
    
    // Read controller data with proper button mapping
    int read_data(int port, SceCtrlData *data, int count);
    
    // Set custom button mappings between Switch and Vita
    int set_button_mapping(int port, int buttons, int mapped_buttons);
    
    // Update controller state from Switch inputs
    void update_state(uint32_t buttons, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry);
};
```

**SceTouch Implementation:**
```cpp
class SceTouchManager {
public:
    // Initialize touch system
    int initialize();
    
    // Read touch data with hybrid rear touch handling
    int read_data(int port, SceTouchData *data);
    
    // Update touch state with rear touch detection
    void update_state(int x, int y, bool pressed, uint32_t controller_buttons);
    
    // Toggle between front/rear touch modes
    void toggle_rear_touch_mode();
    
    // Check if rear touch trigger buttons are pressed
    bool should_activate_rear_touch(uint32_t controller_buttons) const;
};
```

### 4. File System

**Original Implementation:**
- Vita3K virtualizes PS Vita file system
- Uses host OS file system for storage

**Switch Adaptation:**
- Use Switch SD card for storage
- Maintain PS Vita file system structure
- Implement efficient file access

**Implementation Plan:**
```cpp
// Switch file system implementation
void init_switch_filesystem() {
    // Mount SD card
    // Set up virtual file system paths
    // Create necessary directories
}
```

### 5. User Interface

**Original Implementation:**
- Vita3K uses ImGui for interface
- Designed for desktop environments

**Switch Adaptation:**
- Adapt UI for Switch screen size
- Implement touch-friendly controls
- Create Switch-specific settings

**Implementation Plan:**
```cpp
// Switch UI adaptation
void init_switch_ui() {
    // Initialize ImGui for Switch
    // Set up touch input for UI
    // Create Switch-specific UI components
}
```

### 6. PS Vita OS Advanced Features

#### 6.1 Trophy System

**Original Implementation:**
- PS Vita provides trophy support through SceNpTrophy module
- Trophies are stored in a secure database

**Switch Adaptation:**
- Implement SceTrophyManager for trophy handling
- Use JSON for trophy data storage
- Implement trophy unlocking and status tracking

**Implementation Details:**
```cpp
// Trophy system implementation
class SceTrophyManager {
public:
    // Initialize trophy system
    int initialize();
    
    // Create trophy context
    int createContext(uint32_t* context_id, const char* comm_id, const char* comm_sign);
    
    // Unlock a trophy
    int unlockTrophy(uint32_t context_id, uint32_t handle_id, uint32_t trophy_id);
    
    // Get trophy information
    int getTrophyInfo(uint32_t context_id, uint32_t handle_id, uint32_t trophy_id, SceTrophyInfo* info);
    
    // Terminate trophy system
    int terminate();
};
```

#### 6.2 Camera Emulation

**Original Implementation:**
- PS Vita has front and rear cameras
- SceCamera module provides camera control

**Switch Adaptation:**
- Implement SceCameraManager for camera device management
- Create CameraDevice class for camera operations
- Generate test frames for camera simulation
- Provide hooks for Switch hardware camera integration

**Implementation Details:**
```cpp
// Camera emulation system
class SceCameraManager {
public:
    // Initialize camera system
    int initialize();
    
    // Open camera device
    int openCamera(int device_index, SceCameraInfo* info);
    
    // Start camera capture
    int startCamera(int device_handle);
    
    // Read camera frame
    int readFrame(int device_handle, SceCameraFrame* frame);
    
    // Stop camera
    int stopCamera(int device_handle);
    
    // Close camera
    int closeCamera(int device_handle);
    
    // Terminate camera system
    int terminate();
};
```

#### 6.3 Power Management

**Original Implementation:**
- PS Vita provides power management through ScePower module
- Handles battery status, CPU/GPU frequency, and power events

**Switch Adaptation:**
- Implement ScePowerManager for power-related functionality
- Use libnx for accessing Switch battery and power information
- Implement power callbacks and event handling

**Implementation Details:**
```cpp
// Power management system
class ScePowerManager {
public:
    // Initialize power management
    int initialize();
    
    // Get battery percentage
    int getBatteryLifePercent();
    
    // Get battery remaining time
    int getBatteryLifeTime();
    
    // Register power callback
    int registerCallback(int slot, int cbid);
    
    // Unregister power callback
    int unregisterCallback(int slot);
    
    // Set CPU frequency
    int setCpuFrequency(int freq);
    
    // Get CPU frequency
    int getCpuFrequency();
    
    // Terminate power management
    int terminate();
};
```

## Integration with Vita3K Codebase

### Approach 1: Direct Integration

Integrate Switch-specific code directly into the Vita3K codebase using preprocessor directives:

```cpp
#ifdef __SWITCH__
    // Switch-specific implementation
#else
    // Original implementation
#endif
```

**Pros:**
- Single codebase
- Easier to merge upstream changes

**Cons:**
- More complex build system
- Potential for code clutter

### Approach 2: Platform Layer

Create a platform abstraction layer that isolates platform-specific code:

```cpp
// Platform interface
class Platform {
public:
    virtual bool initialize() = 0;
    virtual void finalize() = 0;
    // ...
};

// Switch implementation
class SwitchPlatform : public Platform {
public:
    virtual bool initialize() override;
    virtual void finalize() override;
    // ...
};
```

**Pros:**
- Cleaner code separation
- Easier to maintain
- More extensible

**Cons:**
- Initial development overhead
- Potential performance impact

**Selected Approach:** Platform Layer

## Performance Optimization Strategy

### 1. CPU Optimization

- Utilize ARM-to-ARM translation efficiencies
- Implement JIT caching optimized for Switch
- Profile and optimize hot paths

### 2. GPU Optimization

- Batch rendering commands
- Optimize shader compilation
- Implement texture caching
- Use resolution scaling based on performance

### 3. Memory Optimization

- Implement efficient memory allocation
- Use memory pooling for frequent allocations
- Optimize texture memory usage

### 4. I/O Optimization

- Cache frequently accessed files
- Implement asynchronous loading
- Optimize SD card access patterns

## Testing Strategy

### 1. Unit Testing

- Implement unit tests for core components
- Automate testing where possible

### 2. Integration Testing

- Test component interactions
- Verify system behavior

### 3. Game Compatibility Testing

- Create a test suite with representative games
- Track compatibility status
- Implement game-specific fixes

### 4. Performance Testing

- Benchmark CPU usage
- Measure frame rates
- Monitor memory usage
- Track loading times

## Development Roadmap

### Milestone 1: Basic Framework

- Switch homebrew application structure
- SDL2 initialization
- Basic rendering

### Milestone 2: Core Systems

- Memory management
- CPU emulation
- File system access
- Basic input handling

### Milestone 3: Graphics and Audio

- OpenGL ES renderer
- Shader translation
- Audio playback
- Input mapping

### Milestone 4: UI and Polish

- User interface
- Settings menu
- Performance optimizations
- Game compatibility improvements

## Conclusion

This technical design document outlines the approach for porting Vita3K to the Nintendo Switch. By following this architecture and implementation plan, we can create an efficient and functional PS Vita emulator for the Switch homebrew environment.
