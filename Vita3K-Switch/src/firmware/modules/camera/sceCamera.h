#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <atomic>

namespace firmware {
namespace modules {
namespace camera {

// Camera error codes
enum SceCameraErrorCode {
    SCE_CAMERA_ERROR_NONE                = 0,
    SCE_CAMERA_ERROR_NOT_INITIALIZED     = 0x80290101,
    SCE_CAMERA_ERROR_ALREADY_INITIALIZED = 0x80290102,
    SCE_CAMERA_ERROR_NOT_OPEN            = 0x80290103,
    SCE_CAMERA_ERROR_ALREADY_OPEN        = 0x80290104,
    SCE_CAMERA_ERROR_DEVICE_NOT_FOUND    = 0x80290105,
    SCE_CAMERA_ERROR_NOT_SUPPORTED       = 0x80290106,
    SCE_CAMERA_ERROR_INVALID_PARAM       = 0x80290107,
    SCE_CAMERA_ERROR_INVALID_HANDLE      = 0x80290108,
    SCE_CAMERA_ERROR_INVALID_FRAME_DATA  = 0x80290109,
    SCE_CAMERA_ERROR_INVALID_FORMAT      = 0x8029010A,
    SCE_CAMERA_ERROR_INVALID_RESOLUTION  = 0x8029010B,
    SCE_CAMERA_ERROR_INVALID_FRAMERATE   = 0x8029010C,
    SCE_CAMERA_ERROR_INVALID_SATURATION  = 0x8029010D,
    SCE_CAMERA_ERROR_INVALID_BRIGHTNESS  = 0x8029010E,
    SCE_CAMERA_ERROR_INVALID_CONTRAST    = 0x8029010F,
    SCE_CAMERA_ERROR_INVALID_SHARPNESS   = 0x80290110,
    SCE_CAMERA_ERROR_INVALID_ZOOM        = 0x80290111,
    SCE_CAMERA_ERROR_INVALID_EFFECT      = 0x80290112,
    SCE_CAMERA_ERROR_INVALID_EXPOSURE    = 0x80290113,
    SCE_CAMERA_ERROR_INVALID_WHITE_BALANCE = 0x80290114,
    SCE_CAMERA_ERROR_INVALID_REVERSE     = 0x80290115,
    SCE_CAMERA_ERROR_NO_DEVICE_MEMORY    = 0x80290116,
    SCE_CAMERA_ERROR_FATAL               = 0x802901FF
};

// Camera constants
#define SCE_CAMERA_MAX_DEVICES           2
#define SCE_CAMERA_DEVICE_FRONT          0
#define SCE_CAMERA_DEVICE_BACK           1

#define SCE_CAMERA_FORMAT_YUVU422        0
#define SCE_CAMERA_FORMAT_YUV422         1
#define SCE_CAMERA_FORMAT_YUV420         2
#define SCE_CAMERA_FORMAT_ARGB           3
#define SCE_CAMERA_FORMAT_ABGR           4
#define SCE_CAMERA_FORMAT_RAW8           5

#define SCE_CAMERA_RESOLUTION_640_480    1
#define SCE_CAMERA_RESOLUTION_320_240    2
#define SCE_CAMERA_RESOLUTION_160_120    3
#define SCE_CAMERA_RESOLUTION_352_288    4
#define SCE_CAMERA_RESOLUTION_176_144    5
#define SCE_CAMERA_RESOLUTION_480_272    6
#define SCE_CAMERA_RESOLUTION_640_360    7

#define SCE_CAMERA_FRAMERATE_3_FPS       1
#define SCE_CAMERA_FRAMERATE_5_FPS       2
#define SCE_CAMERA_FRAMERATE_7_5_FPS     3
#define SCE_CAMERA_FRAMERATE_10_FPS      4
#define SCE_CAMERA_FRAMERATE_15_FPS      5
#define SCE_CAMERA_FRAMERATE_20_FPS      6
#define SCE_CAMERA_FRAMERATE_30_FPS      7
#define SCE_CAMERA_FRAMERATE_60_FPS      8

#define SCE_CAMERA_EFFECT_NONE           0
#define SCE_CAMERA_EFFECT_NEGATIVE       1
#define SCE_CAMERA_EFFECT_BLACKWHITE     2
#define SCE_CAMERA_EFFECT_SEPIA          3
#define SCE_CAMERA_EFFECT_BLUE           4
#define SCE_CAMERA_EFFECT_RED            5
#define SCE_CAMERA_EFFECT_GREEN          6

#define SCE_CAMERA_WHITE_BALANCE_AUTO    0
#define SCE_CAMERA_WHITE_BALANCE_DAYLIGHT 1
#define SCE_CAMERA_WHITE_BALANCE_FLUORESCENT 2
#define SCE_CAMERA_WHITE_BALANCE_INCANDESCENT 3
#define SCE_CAMERA_WHITE_BALANCE_WARM_FLUORESCENT 4
#define SCE_CAMERA_WHITE_BALANCE_CLOUDY  5

#define SCE_CAMERA_EXPOSURE_AUTO         0
#define SCE_CAMERA_EXPOSURE_MANUAL       1

#define SCE_CAMERA_INFO_ACTIVE           1
#define SCE_CAMERA_INFO_RESOLUTION       2
#define SCE_CAMERA_INFO_FRAMERATE        3
#define SCE_CAMERA_INFO_FORMAT           4
#define SCE_CAMERA_INFO_SATURATION       5
#define SCE_CAMERA_INFO_BRIGHTNESS       6
#define SCE_CAMERA_INFO_CONTRAST         7
#define SCE_CAMERA_INFO_SHARPNESS        8
#define SCE_CAMERA_INFO_ZOOM             9
#define SCE_CAMERA_INFO_EFFECT           10
#define SCE_CAMERA_INFO_EXPOSURE         11
#define SCE_CAMERA_INFO_WHITE_BALANCE    12
#define SCE_CAMERA_INFO_REVERSE          13
#define SCE_CAMERA_INFO_DEVICE_LOCATION  14

// Camera structures
struct SceCameraInfo {
    uint32_t size;
    uint32_t device_id;
    uint32_t resolution;
    uint32_t framerate;
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t range;
    uint32_t pad[5];
};

struct SceCameraRead {
    uint32_t size;
    uint32_t frame_timestamp;
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t frame_format;
    uint32_t frame_size;
    void* frame_data;
    uint32_t pad[5];
};

struct SceCameraDeviceInfo {
    uint32_t size;
    uint32_t device_id;
    uint32_t status;
    uint32_t pad[5];
};

// Camera device class
class CameraDevice {
public:
    CameraDevice(int device_id);
    ~CameraDevice();

    // Open the camera device
    int open();

    // Close the camera device
    int close();

    // Start the camera device
    int start();

    // Stop the camera device
    int stop();

    // Read a frame from the camera
    int read(SceCameraRead* read);

    // Set camera parameters
    int set_parameter(int param_type, int value);

    // Get camera parameters
    int get_parameter(int param_type, int* value);

    // Get camera information
    int get_info(SceCameraInfo* info);

    // Check if camera is open
    bool is_open() const;

    // Check if camera is active (started)
    bool is_active() const;

private:
    // Device ID (0 = front, 1 = back)
    int device_id;

    // Device state
    bool opened;
    bool active;

    // Camera parameters
    int resolution;
    int framerate;
    int format;
    int saturation;
    int brightness;
    int contrast;
    int sharpness;
    int zoom;
    int effect;
    int exposure;
    int white_balance;
    int reverse;

    // Frame dimensions
    int width;
    int height;
    int frame_size;

    // Frame buffer
    std::vector<uint8_t> frame_buffer;

    // Frame timestamp
    uint32_t frame_timestamp;

    // Camera thread
    std::unique_ptr<std::thread> camera_thread;
    std::atomic<bool> thread_running;

    // Mutex for thread safety
    mutable std::mutex mutex;

    // Camera thread function
    void camera_thread_func();

    // Generate a test pattern frame
    void generate_test_frame();

    // Helper functions
    bool is_valid_resolution(int resolution) const;
    bool is_valid_framerate(int framerate) const;
    bool is_valid_format(int format) const;
    bool is_valid_effect(int effect) const;
    bool is_valid_white_balance(int white_balance) const;
    bool is_valid_exposure(int exposure) const;
    void update_frame_dimensions();
};

// Camera manager class
class SceCameraManager {
public:
    SceCameraManager();
    ~SceCameraManager();

    // Initialize the camera system
    int initialize();

    // Terminate the camera system
    int terminate();

    // Open a camera device
    int open(int device_id);

    // Close a camera device
    int close(int device_id);

    // Start a camera device
    int start(int device_id);

    // Stop a camera device
    int stop(int device_id);

    // Read a frame from a camera device
    int read(int device_id, SceCameraRead* read);

    // Set camera parameters
    int set_parameter(int device_id, int param_type, int value);

    // Get camera parameters
    int get_parameter(int device_id, int param_type, int* value);

    // Get camera information
    int get_info(int device_id, SceCameraInfo* info);

    // Get camera device information
    int get_device_info(SceCameraDeviceInfo* info);

private:
    // Camera initialized flag
    bool initialized;

    // Camera devices
    std::map<int, std::unique_ptr<CameraDevice>> devices;

    // Mutex for thread safety
    mutable std::mutex mutex;

    // Helper functions
    bool is_valid_device_id(int device_id) const;
};

// Module functions
int sceCameraInit();
int sceCameraExit();
int sceCameraOpen(int device_id);
int sceCameraClose(int device_id);
int sceCameraStart(int device_id);
int sceCameraStop(int device_id);
int sceCameraRead(int device_id, SceCameraRead* read);
int sceCameraSetParameter(int device_id, int param_type, int value);
int sceCameraGetParameter(int device_id, int param_type, int* value);
int sceCameraGetInfo(int device_id, SceCameraInfo* info);
int sceCameraGetDeviceInfo(SceCameraDeviceInfo* info);

// Forward declaration for framerate helper
int get_fps_from_framerate(int framerate);

} // namespace camera
} // namespace modules
} // namespace firmware
