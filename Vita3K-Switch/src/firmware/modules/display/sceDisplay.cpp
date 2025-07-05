#include "sceDisplay.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>

namespace firmware {
namespace modules {
namespace display {

// Global display manager instance
static SceDisplayManager g_display_manager;

// SceDisplayManager implementation
SceDisplayManager::SceDisplayManager()
    : initialized(false), 
      rear_touch_overlay_callback(nullptr), 
      rear_touch_overlay_user_data(nullptr),
      show_overlay(false),
      touch_x(0),
      touch_y(0) {
    memset(&frame_buf, 0, sizeof(SceDisplayFrameBuf));
}

SceDisplayManager::~SceDisplayManager() {
    if (initialized) {
        terminate();
    }
}

int SceDisplayManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (initialized) {
        std::cerr << "Display manager is already initialized" << std::endl;
        return SCE_DISPLAY_ERROR_ALREADY_INITIALIZED;
    }
    
    std::cout << "Initializing display manager" << std::endl;
    
    // Initialize frame buffer
    memset(&frame_buf, 0, sizeof(SceDisplayFrameBuf));
    
    initialized = true;
    std::cout << "Display manager initialized successfully" << std::endl;
    return SCE_DISPLAY_ERROR_OK;
}

int SceDisplayManager::terminate() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Display manager is not initialized" << std::endl;
        return SCE_DISPLAY_ERROR_NOT_INITIALIZED;
    }
    
    std::cout << "Terminating display manager" << std::endl;
    
    // Clear frame buffer
    memset(&frame_buf, 0, sizeof(SceDisplayFrameBuf));
    
    initialized = false;
    std::cout << "Display manager terminated successfully" << std::endl;
    return SCE_DISPLAY_ERROR_OK;
}

int SceDisplayManager::set_frame_buf(const SceDisplayFrameBuf *pFrameBuf, int updateTiming) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Display manager is not initialized" << std::endl;
        return SCE_DISPLAY_ERROR_NOT_INITIALIZED;
    }
    
    if (!pFrameBuf) {
        std::cerr << "Invalid frame buffer pointer" << std::endl;
        return SCE_DISPLAY_ERROR_INVALID_ADDR;
    }
    
    if (pFrameBuf->size != sizeof(SceDisplayFrameBuf)) {
        std::cerr << "Invalid frame buffer size: " << pFrameBuf->size << std::endl;
        return SCE_DISPLAY_ERROR_INVALID_VALUE;
    }
    
    if (!pFrameBuf->base) {
        std::cerr << "Invalid frame buffer base address" << std::endl;
        return SCE_DISPLAY_ERROR_INVALID_ADDR;
    }
    
    if (pFrameBuf->width == 0 || pFrameBuf->height == 0) {
        std::cerr << "Invalid frame buffer dimensions: " << pFrameBuf->width << "x" << pFrameBuf->height << std::endl;
        return SCE_DISPLAY_ERROR_INVALID_RESOLUTION;
    }
    
    if (pFrameBuf->pitch == 0) {
        std::cerr << "Invalid frame buffer pitch: " << pFrameBuf->pitch << std::endl;
        return SCE_DISPLAY_ERROR_INVALID_PITCH;
    }
    
    if (pFrameBuf->pixelformat != SCE_DISPLAY_PIXELFORMAT_A8B8G8R8 && 
        pFrameBuf->pixelformat != SCE_DISPLAY_PIXELFORMAT_A8R8G8B8) {
        std::cerr << "Invalid frame buffer pixel format: " << pFrameBuf->pixelformat << std::endl;
        return SCE_DISPLAY_ERROR_INVALID_PIXELFORMAT;
    }
    
    if (updateTiming != SCE_DISPLAY_UPDATETIMING_NEXTVSYNC && 
        updateTiming != SCE_DISPLAY_UPDATETIMING_IMMEDIATE) {
        std::cerr << "Invalid update timing: " << updateTiming << std::endl;
        return SCE_DISPLAY_ERROR_INVALID_UPDATETIMING;
    }
    
    // Copy frame buffer info
    memcpy(&frame_buf, pFrameBuf, sizeof(SceDisplayFrameBuf));
    
    // If we have a rear touch overlay callback and the overlay is visible,
    // call it to draw the overlay on the new frame buffer
    if (rear_touch_overlay_callback && show_overlay) {
        rear_touch_overlay_callback(rear_touch_overlay_user_data, 
                                   (uintptr_t)frame_buf.base, 
                                   frame_buf.pitch, 
                                   touch_x, 
                                   touch_y);
    }
    
    return SCE_DISPLAY_ERROR_OK;
}

int SceDisplayManager::get_frame_buf(SceDisplayFrameBuf *pFrameBuf) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Display manager is not initialized" << std::endl;
        return SCE_DISPLAY_ERROR_NOT_INITIALIZED;
    }
    
    if (!pFrameBuf) {
        std::cerr << "Invalid frame buffer pointer" << std::endl;
        return SCE_DISPLAY_ERROR_INVALID_ADDR;
    }
    
    if (pFrameBuf->size != sizeof(SceDisplayFrameBuf)) {
        std::cerr << "Invalid frame buffer size: " << pFrameBuf->size << std::endl;
        return SCE_DISPLAY_ERROR_INVALID_VALUE;
    }
    
    // Check if frame buffer is set
    if (!frame_buf.base) {
        std::cerr << "No frame buffer set" << std::endl;
        return SCE_DISPLAY_ERROR_NO_FRAME_BUFFER;
    }
    
    // Copy frame buffer info
    memcpy(pFrameBuf, &frame_buf, sizeof(SceDisplayFrameBuf));
    
    return SCE_DISPLAY_ERROR_OK;
}

int SceDisplayManager::wait_vblank() {
    if (!initialized) {
        std::cerr << "Display manager is not initialized" << std::endl;
        return SCE_DISPLAY_ERROR_NOT_INITIALIZED;
    }
    
    // Simulate VBlank by sleeping for 16.6ms (60Hz)
    std::this_thread::sleep_for(std::chrono::milliseconds(17));
    
    return SCE_DISPLAY_ERROR_OK;
}

int SceDisplayManager::get_refresh_rate(float *pRate) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "Display manager is not initialized" << std::endl;
        return SCE_DISPLAY_ERROR_NOT_INITIALIZED;
    }
    
    if (!pRate) {
        std::cerr << "Invalid rate pointer" << std::endl;
        return SCE_DISPLAY_ERROR_INVALID_ADDR;
    }
    
    // PS Vita refresh rate is 59.94Hz
    *pRate = 59.94f;
    
    return SCE_DISPLAY_ERROR_OK;
}

void SceDisplayManager::get_max_resolution(uint32_t *width, uint32_t *height) {
    if (width) {
        *width = 960;  // PS Vita screen width
    }
    
    if (height) {
        *height = 544; // PS Vita screen height
    }
}

void SceDisplayManager::register_rear_touch_overlay_callback(void (*callback)(void*, uint32_t, uint32_t, uint32_t, uint32_t), void* userData) {
    std::lock_guard<std::mutex> lock(mutex);
    
    rear_touch_overlay_callback = callback;
    rear_touch_overlay_user_data = userData;
}

void SceDisplayManager::show_rear_touch_overlay(bool show) {
    std::lock_guard<std::mutex> lock(mutex);
    
    show_overlay = show;
    
    // If showing the overlay and we have a callback, call it
    if (show_overlay && rear_touch_overlay_callback && frame_buf.base) {
        rear_touch_overlay_callback(rear_touch_overlay_user_data, 
                                   (uintptr_t)frame_buf.base, 
                                   frame_buf.pitch, 
                                   touch_x, 
                                   touch_y);
    }
}

void SceDisplayManager::update_rear_touch_position(int x, int y) {
    std::lock_guard<std::mutex> lock(mutex);
    
    touch_x = x;
    touch_y = y;
    
    // If showing the overlay and we have a callback, call it
    if (show_overlay && rear_touch_overlay_callback && frame_buf.base) {
        rear_touch_overlay_callback(rear_touch_overlay_user_data, 
                                   (uintptr_t)frame_buf.base, 
                                   frame_buf.pitch, 
                                   touch_x, 
                                   touch_y);
    }
}

// Module function implementations
int sceDisplayInit() {
    return g_display_manager.initialize();
}

int sceDisplayExit() {
    return g_display_manager.terminate();
}

int sceDisplaySetFrameBuf(const SceDisplayFrameBuf *pFrameBuf, int updateTiming) {
    return g_display_manager.set_frame_buf(pFrameBuf, updateTiming);
}

int sceDisplayGetFrameBuf(SceDisplayFrameBuf *pFrameBuf, int updateTiming) {
    // updateTiming is ignored in this implementation
    return g_display_manager.get_frame_buf(pFrameBuf);
}

int sceDisplayWaitVblankStart() {
    return g_display_manager.wait_vblank();
}

int sceDisplayWaitVblankStartMulti(uint32_t vcount) {
    for (uint32_t i = 0; i < vcount; i++) {
        int ret = g_display_manager.wait_vblank();
        if (ret != SCE_DISPLAY_ERROR_OK) {
            return ret;
        }
    }
    
    return SCE_DISPLAY_ERROR_OK;
}

int sceDisplayWaitVblankStartCB() {
    // Same as non-CB version, as we don't have callbacks yet
    return g_display_manager.wait_vblank();
}

int sceDisplayWaitVblankStartMultiCB(uint32_t vcount) {
    // Same as non-CB version, as we don't have callbacks yet
    return sceDisplayWaitVblankStartMulti(vcount);
}

int sceDisplayWaitSetFrameBuf() {
    // Just wait for VBlank
    return g_display_manager.wait_vblank();
}

int sceDisplayWaitSetFrameBufCB() {
    // Same as non-CB version, as we don't have callbacks yet
    return g_display_manager.wait_vblank();
}

int sceDisplayWaitSetFrameBufMulti(uint32_t vcount) {
    return sceDisplayWaitVblankStartMulti(vcount);
}

int sceDisplayWaitSetFrameBufMultiCB(uint32_t vcount) {
    // Same as non-CB version, as we don't have callbacks yet
    return sceDisplayWaitVblankStartMulti(vcount);
}

int sceDisplayGetRefreshRate(float *pRate) {
    return g_display_manager.get_refresh_rate(pRate);
}

int sceDisplayGetVcount() {
    // Return a dummy VCount
    static uint32_t vcount = 0;
    return vcount++;
}

int sceDisplayRegisterVblankStartCallback(int32_t uid) {
    // Not implemented
    return SCE_DISPLAY_ERROR_OK;
}

int sceDisplayUnregisterVblankStartCallback(int32_t uid) {
    // Not implemented
    return SCE_DISPLAY_ERROR_OK;
}

int sceDisplayGetMaximumFrameBufResolution(uint32_t *width, uint32_t *height) {
    g_display_manager.get_max_resolution(width, height);
    return SCE_DISPLAY_ERROR_OK;
}

// Custom functions for rear touch panel overlay
void sceDisplayRegisterRearTouchOverlayCallback(void (*callback)(void*, uint32_t, uint32_t, uint32_t, uint32_t), void* userData) {
    g_display_manager.register_rear_touch_overlay_callback(callback, userData);
}

void sceDisplayShowRearTouchOverlay(bool show) {
    g_display_manager.show_rear_touch_overlay(show);
}

void sceDisplayUpdateRearTouchPosition(int x, int y) {
    g_display_manager.update_rear_touch_position(x, y);
}

} // namespace display
} // namespace modules
} // namespace firmware
