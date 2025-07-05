#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

namespace firmware {
namespace modules {
namespace display {

// Display error codes
enum SceDisplayErrorCode {
    SCE_DISPLAY_ERROR_OK                    = 0,
    SCE_DISPLAY_ERROR_INVALID_HEAD          = 0x80290000,
    SCE_DISPLAY_ERROR_INVALID_VALUE         = 0x80290001,
    SCE_DISPLAY_ERROR_INVALID_ADDR          = 0x80290002,
    SCE_DISPLAY_ERROR_INVALID_PIXELFORMAT   = 0x80290003,
    SCE_DISPLAY_ERROR_INVALID_PITCH         = 0x80290004,
    SCE_DISPLAY_ERROR_INVALID_RESOLUTION    = 0x80290005,
    SCE_DISPLAY_ERROR_INVALID_UPDATETIMING  = 0x80290006,
    SCE_DISPLAY_ERROR_NO_FRAME_BUFFER      = 0x80290007,
    SCE_DISPLAY_ERROR_NO_PIXEL_DATA        = 0x80290008,
    SCE_DISPLAY_ERROR_ALREADY_INITIALIZED   = 0x80290009,
    SCE_DISPLAY_ERROR_NOT_INITIALIZED       = 0x8029000A
};

// Display constants
#define SCE_DISPLAY_PIXELFORMAT_A8B8G8R8    0x00000000U
#define SCE_DISPLAY_PIXELFORMAT_A8R8G8B8    0x00000001U

#define SCE_DISPLAY_UPDATETIMING_NEXTVSYNC  0
#define SCE_DISPLAY_UPDATETIMING_IMMEDIATE  1

#define SCE_DISPLAY_SETBUF_IMMEDIATE        0
#define SCE_DISPLAY_SETBUF_NEXTFRAME        1

// Display frame buffer info
struct SceDisplayFrameBuf {
    uint32_t size;
    void *base;
    uint32_t pitch;
    uint32_t pixelformat;
    uint32_t width;
    uint32_t height;
};

// Display manager class
class SceDisplayManager {
public:
    SceDisplayManager();
    ~SceDisplayManager();

    // Initialize the display manager
    int initialize();

    // Terminate the display manager
    int terminate();

    // Set frame buffer
    int set_frame_buf(const SceDisplayFrameBuf *pFrameBuf, int updateTiming);

    // Get frame buffer
    int get_frame_buf(SceDisplayFrameBuf *pFrameBuf);

    // Wait for VBlank
    int wait_vblank();

    // Get refresh rate
    int get_refresh_rate(float *pRate);

    // Get maximum resolution
    void get_max_resolution(uint32_t *width, uint32_t *height);

    // Register callback for rear touch panel overlay
    void register_rear_touch_overlay_callback(void (*callback)(void*, uint32_t, uint32_t, uint32_t, uint32_t), void* userData);

    // Show rear touch overlay
    void show_rear_touch_overlay(bool show);

    // Update rear touch position
    void update_rear_touch_position(int x, int y);

private:
    // Frame buffer
    SceDisplayFrameBuf frame_buf;

    // Display initialized flag
    bool initialized;

    // Mutex for thread safety
    mutable std::mutex mutex;

    // Rear touch overlay callback
    void (*rear_touch_overlay_callback)(void*, uint32_t, uint32_t, uint32_t, uint32_t);
    void* rear_touch_overlay_user_data;

    // Rear touch overlay state
    bool show_overlay;
    int touch_x;
    int touch_y;
};

// Module functions
int sceDisplayInit();
int sceDisplayExit();
int sceDisplaySetFrameBuf(const SceDisplayFrameBuf *pFrameBuf, int updateTiming);
int sceDisplayGetFrameBuf(SceDisplayFrameBuf *pFrameBuf, int updateTiming);
int sceDisplayWaitVblankStart();
int sceDisplayWaitVblankStartMulti(uint32_t vcount);
int sceDisplayWaitVblankStartCB();
int sceDisplayWaitVblankStartMultiCB(uint32_t vcount);
int sceDisplayWaitSetFrameBuf();
int sceDisplayWaitSetFrameBufCB();
int sceDisplayWaitSetFrameBufMulti(uint32_t vcount);
int sceDisplayWaitSetFrameBufMultiCB(uint32_t vcount);
int sceDisplayGetRefreshRate(float *pRate);
int sceDisplayGetVcount();
int sceDisplayRegisterVblankStartCallback(int32_t uid);
int sceDisplayUnregisterVblankStartCallback(int32_t uid);
int sceDisplayGetMaximumFrameBufResolution(uint32_t *width, uint32_t *height);

// Custom functions for rear touch panel overlay
void sceDisplayRegisterRearTouchOverlayCallback(void (*callback)(void*, uint32_t, uint32_t, uint32_t, uint32_t), void* userData);
void sceDisplayShowRearTouchOverlay(bool show);
void sceDisplayUpdateRearTouchPosition(int x, int y);

} // namespace display
} // namespace modules
} // namespace firmware
