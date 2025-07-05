#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

namespace firmware {
namespace modules {
namespace input {

// Controller error codes
enum SceCtrlErrorCode {
    SCE_CTRL_ERROR_INVALID_ARG        = 0x80340001,
    SCE_CTRL_ERROR_PRIV_REQUIRED      = 0x80340002,
    SCE_CTRL_ERROR_NO_DEVICE          = 0x80340003,
    SCE_CTRL_ERROR_NOT_SUPPORTED      = 0x80340004,
    SCE_CTRL_ERROR_INVALID_MODE       = 0x80340005,
    SCE_CTRL_ERROR_INVALID_TYPE       = 0x80340006,
    SCE_CTRL_ERROR_ALREADY_INIT       = 0x80340007,
    SCE_CTRL_ERROR_NOT_INIT           = 0x80340008,
    SCE_CTRL_ERROR_FATAL              = 0x803400FF
};

// Controller button masks
#define SCE_CTRL_SELECT      0x00000001
#define SCE_CTRL_L3          0x00000002
#define SCE_CTRL_R3          0x00000004
#define SCE_CTRL_START       0x00000008
#define SCE_CTRL_UP          0x00000010
#define SCE_CTRL_RIGHT       0x00000020
#define SCE_CTRL_DOWN        0x00000040
#define SCE_CTRL_LEFT        0x00000080
#define SCE_CTRL_LTRIGGER    0x00000100
#define SCE_CTRL_RTRIGGER    0x00000200
#define SCE_CTRL_L1          0x00000400
#define SCE_CTRL_R1          0x00000800
#define SCE_CTRL_TRIANGLE    0x00001000
#define SCE_CTRL_CIRCLE      0x00002000
#define SCE_CTRL_CROSS       0x00004000
#define SCE_CTRL_SQUARE      0x00008000
#define SCE_CTRL_ANY         0x0000FFFF

// Controller mode
#define SCE_CTRL_MODE_DIGITAL        0
#define SCE_CTRL_MODE_ANALOG         1
#define SCE_CTRL_MODE_ANALOG_WIDE    2

// Controller type
#define SCE_CTRL_TYPE_UNPAIRED       0
#define SCE_CTRL_TYPE_PHY            1
#define SCE_CTRL_TYPE_VIRT           2
#define SCE_CTRL_TYPE_DS3            3
#define SCE_CTRL_TYPE_DS4            4

// Controller port
#define SCE_CTRL_PORT_TYPE_STANDARD  0
#define SCE_CTRL_PORT_TYPE_UNKNOWN   1
#define SCE_CTRL_PORT_TYPE_REMOTE    2
#define SCE_CTRL_PORT_TYPE_DS3       3
#define SCE_CTRL_PORT_TYPE_DS4       4

// Controller data structure
struct SceCtrlData {
    uint64_t timeStamp;
    uint32_t buttons;
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
    uint8_t reserved[16];
};

// Controller stick data structure
struct SceCtrlStickData {
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
};

// Controller analog stick calibration data
struct SceCtrlCalibrationData {
    uint8_t lx_max;
    uint8_t ly_max;
    uint8_t rx_max;
    uint8_t ry_max;
    uint8_t lx_min;
    uint8_t ly_min;
    uint8_t rx_min;
    uint8_t ry_min;
};

// Controller manager class
class SceCtrlManager {
public:
    SceCtrlManager();
    ~SceCtrlManager();

    // Initialize the controller manager
    int initialize();

    // Terminate the controller manager
    int terminate();

    // Set controller mode
    int set_mode(int port, int mode);

    // Get controller mode
    int get_mode(int port, int *mode);

    // Set controller sampling rate
    int set_sampling_rate(int port, int rate);

    // Get controller sampling rate
    int get_sampling_rate(int port, int *rate);

    // Set controller button mapping
    int set_button_mapping(int port, int buttons, int mapped_buttons);

    // Read controller data
    int read_data(int port, SceCtrlData *data, int count);

    // Peek controller data
    int peek_data(int port, SceCtrlData *data, int count);

    // Read controller buffer data
    int read_buffer_data(int port, SceCtrlData *data, int count);

    // Set controller actuator
    int set_actuator(int port, int small, int large);

    // Get controller information
    int get_controller_info(int port, int *type, int *status);

    // Get controller battery level
    int get_battery_level(int port, int *level);

    // Update controller state (called by the system)
    void update_state(uint32_t buttons, uint8_t lx, uint8_t ly, uint8_t rx, uint8_t ry);

private:
    // Controller state
    struct ControllerState {
        uint32_t buttons;
        uint8_t lx;
        uint8_t ly;
        uint8_t rx;
        uint8_t ry;
        int mode;
        int sampling_rate;
        int type;
        int status;
        int battery_level;
        std::vector<SceCtrlData> buffer;
    };

    // Controller states
    std::map<int, ControllerState> controllers;

    // Button mapping
    std::map<int, std::map<int, int>> button_mappings;

    // Controller initialized flag
    bool initialized;

    // Mutex for thread safety
    mutable std::mutex mutex;

    // Get current timestamp
    uint64_t get_timestamp() const;

    // Map Switch buttons to PS Vita buttons
    uint32_t map_buttons(int port, uint32_t buttons) const;
};

// Module functions
int sceCtrlInit();
int sceCtrlExit();
int sceCtrlSetSamplingMode(int mode);
int sceCtrlGetSamplingMode(int *mode);
int sceCtrlSetSamplingModeExt(int port, int mode);
int sceCtrlGetSamplingModeExt(int port, int *mode);
int sceCtrlReadBufferPositive(int port, SceCtrlData *data, int count);
int sceCtrlReadBufferNegative(int port, SceCtrlData *data, int count);
int sceCtrlPeekBufferPositive(int port, SceCtrlData *data, int count);
int sceCtrlPeekBufferNegative(int port, SceCtrlData *data, int count);
int sceCtrlReadBuffer(int port, SceCtrlData *data, int count);
int sceCtrlPeekBuffer(int port, SceCtrlData *data, int count);
int sceCtrlSetActuator(int port, int small, int large);
int sceCtrlGetControllerPortInfo(int *info);
int sceCtrlGetBatteryInfo(int port, int *level);
int sceCtrlSetButtonMapping(int port, int buttons, int mapped_buttons);
int sceCtrlGetButtonMapping(int port, int buttons, int *mapped_buttons);
int sceCtrlSetRapidFire(int port, int mask, int trigger, int delay, int make, int break_time);
int sceCtrlGetRapidFire(int port, int mask, int *trigger, int *delay, int *make, int *break_time);

} // namespace input
} // namespace modules
} // namespace firmware
