#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

namespace firmware {
namespace modules {
namespace input {

// Touch error codes
enum SceTouchErrorCode {
    SCE_TOUCH_ERROR_INVALID_ARG       = 0x80350001,
    SCE_TOUCH_ERROR_PRIV_REQUIRED     = 0x80350002,
    SCE_TOUCH_ERROR_FATAL             = 0x803500FF
};

// Touch port
#define SCE_TOUCH_PORT_FRONT 0
#define SCE_TOUCH_PORT_BACK  1
#define SCE_TOUCH_PORT_MAX   2

// Touch sampling state
#define SCE_TOUCH_SAMPLING_STATE_STOP 0
#define SCE_TOUCH_SAMPLING_STATE_START 1

// Touch report mode
#define SCE_TOUCH_REPORT_SENSITIVE_MODE 0
#define SCE_TOUCH_REPORT_MAX_REPORT_MODE 1

// Touch report max
#define SCE_TOUCH_MAX_REPORT 8

// Touch data structure
struct SceTouchData {
    uint64_t timeStamp;
    uint32_t status;
    uint32_t reportNum;
    struct {
        uint8_t id;
        uint8_t reserved[3];
        uint16_t x;
        uint16_t y;
        uint16_t z;
        uint16_t reserved2;
    } report[SCE_TOUCH_MAX_REPORT];
};

// Touch panel info structure
struct SceTouchPanelInfo {
    uint16_t minAaX;
    uint16_t minAaY;
    uint16_t maxAaX;
    uint16_t maxAaY;
    uint16_t minDispX;
    uint16_t minDispY;
    uint16_t maxDispX;
    uint16_t maxDispY;
    uint8_t minForce;
    uint8_t maxForce;
    uint8_t reserved[30];
};

// Touch mode
enum SceTouchMode {
    SCE_TOUCH_MODE_NORMAL = 0,
    SCE_TOUCH_MODE_EMULATION = 1,
    SCE_TOUCH_MODE_EMULATION_VIRTUAL_BACK = 2  // Custom mode for our emulation
};

// Touch manager class
class SceTouchManager {
public:
    SceTouchManager();
    ~SceTouchManager();

    // Initialize the touch manager
    int initialize();

    // Terminate the touch manager
    int terminate();

    // Set touch mode
    int set_mode(int port, SceTouchMode mode);

    // Get touch mode
    int get_mode(int port, SceTouchMode *mode);

    // Set sampling state
    int set_sampling_state(int port, int state);

    // Get sampling state
    int get_sampling_state(int port, int *state);

    // Read touch data
    int read_data(int port, SceTouchData *data);

    // Peek touch data
    int peek_data(int port, SceTouchData *data);

    // Get panel info
    int get_panel_info(int port, SceTouchPanelInfo *info);

    // Update touch state (called by the system)
    void update_state(int x, int y, bool pressed, uint32_t controller_buttons);

    // Toggle rear touch mode
    void toggle_rear_touch_mode();

    // Check if rear touch is active
    bool is_rear_touch_active() const;

private:
    // Touch state
    struct TouchState {
        SceTouchMode mode;
        int sampling_state;
        SceTouchData data;
        SceTouchPanelInfo panel_info;
        std::vector<SceTouchData> buffer;
    };

    // Touch states
    std::map<int, TouchState> touch_states;

    // Touch initialized flag
    bool initialized;

    // Rear touch mode flag
    bool rear_touch_mode;

    // Rear touch trigger buttons
    uint32_t rear_touch_trigger_buttons;

    // Mutex for thread safety
    mutable std::mutex mutex;

    // Get current timestamp
    uint64_t get_timestamp() const;

    // Check if rear touch should be active based on controller buttons
    bool should_activate_rear_touch(uint32_t controller_buttons) const;
};

// Module functions
int sceTouchInit();
int sceTouchExit();
int sceTouchSetMode(int port, SceTouchMode mode);
int sceTouchGetMode(int port, SceTouchMode *mode);
int sceTouchSetSamplingState(int port, int state);
int sceTouchGetSamplingState(int port, int *state);
int sceTouchRead(int port, SceTouchData *data, int count);
int sceTouchPeek(int port, SceTouchData *data, int count);
int sceTouchGetPanelInfo(int port, SceTouchPanelInfo *info);
int sceTouchEnableTouchForce(int port);
int sceTouchDisableTouchForce(int port);
int sceTouchGetProcessInfo(int pid, int *process_info);

} // namespace input
} // namespace modules
} // namespace firmware
