#pragma once

#include <cstdint>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <vector>

namespace firmware {
namespace modules {
namespace power {

// Power error codes
enum ScePowerErrorCode {
    SCE_POWER_ERROR_NONE                 = 0,
    SCE_POWER_ERROR_INVALID_VALUE        = 0x802B0100,
    SCE_POWER_ERROR_ALREADY_REGISTERED   = 0x802B0101,
    SCE_POWER_ERROR_NOT_REGISTERED       = 0x802B0102,
    SCE_POWER_ERROR_CALLBACK_NOT_FOUND   = 0x802B0103,
    SCE_POWER_ERROR_INVALID_FREQUENCY    = 0x802B0104,
    SCE_POWER_ERROR_INVALID_PARAM        = 0x802B0105,
    SCE_POWER_ERROR_UNSUPPORTED          = 0x802B0106
};

// Power constants
#define SCE_POWER_CB_BATTERY_CHANGE      0x00000001
#define SCE_POWER_CB_AC_POWER_CHANGE     0x00000002
#define SCE_POWER_CB_LOWBATT             0x00000004
#define SCE_POWER_CB_POWER_SWITCH        0x00000008
#define SCE_POWER_CB_SUSPENDING          0x00000010
#define SCE_POWER_CB_RESUMING            0x00000020
#define SCE_POWER_CB_SHUTDOWN            0x00000040
#define SCE_POWER_CB_UNK_1               0x00000080
#define SCE_POWER_CB_UNK_2               0x00000100
#define SCE_POWER_CB_UNK_3               0x00000200
#define SCE_POWER_CB_UNK_4               0x00000400

#define SCE_POWER_TICK_DEFAULT           0
#define SCE_POWER_TICK_DISABLE_AUTO_SUSPEND 1
#define SCE_POWER_TICK_DISABLE_OLED_OFF  4
#define SCE_POWER_TICK_DISABLE_OLED_DIMMING 6

// Battery status
#define SCE_POWER_BATTERY_ONLINE         0x00000001
#define SCE_POWER_BATTERY_PRESENT        0x00000002
#define SCE_POWER_BATTERY_CHARGING       0x00000004
#define SCE_POWER_BATTERY_LOW            0x00000008

// CPU clock frequencies
#define SCE_POWER_CPU_FREQ_MIN           111
#define SCE_POWER_CPU_FREQ_MAX           444

// Bus clock frequencies
#define SCE_POWER_BUS_FREQ_MIN           111
#define SCE_POWER_BUS_FREQ_MAX           222

// GPU clock frequencies
#define SCE_POWER_GPU_FREQ_MIN           41
#define SCE_POWER_GPU_FREQ_MAX           166

// Power callback function type
typedef void (*ScePowerCallback)(int notifyId, int notifyCount, int powerInfo);

// Power manager class
class ScePowerManager {
public:
    ScePowerManager();
    ~ScePowerManager();

    // Initialize the power manager
    int initialize();

    // Terminate the power manager
    int terminate();

    // Register a callback
    int register_callback(int cbId, ScePowerCallback callback, void* userData);

    // Unregister a callback
    int unregister_callback(int cbId);

    // Get battery remaining capacity (percent)
    int get_battery_life_percent();

    // Get battery remaining time (minutes)
    int get_battery_life_time();

    // Get battery temperature
    int get_battery_temp();

    // Get battery voltage
    int get_battery_volt();

    // Get battery status
    int get_battery_status();

    // Check if battery is charging
    bool is_battery_charging();

    // Check if power is supplied by external source
    bool is_power_supplied();

    // Get CPU clock frequency (MHz)
    int get_cpu_freq();

    // Set CPU clock frequency (MHz)
    int set_cpu_freq(int freq);

    // Get BUS clock frequency (MHz)
    int get_bus_freq();

    // Set BUS clock frequency (MHz)
    int set_bus_freq(int freq);

    // Get GPU clock frequency (MHz)
    int get_gpu_freq();

    // Set GPU clock frequency (MHz)
    int set_gpu_freq(int freq);

    // Request cold reset
    int cold_reset();

    // Request standby
    int request_standby();

    // Request suspend
    int request_suspend();

    // Power tick
    int power_tick(int type);

    // Simulate power events for testing
    void simulate_power_event(int event_type);

private:
    // Power manager initialized flag
    bool initialized;

    // Current CPU frequency (MHz)
    int cpu_freq;

    // Current BUS frequency (MHz)
    int bus_freq;

    // Current GPU frequency (MHz)
    int gpu_freq;

    // Battery status
    int battery_status;

    // Battery capacity (percent)
    int battery_percent;

    // Battery remaining time (minutes)
    int battery_time;

    // Battery temperature (degrees C)
    int battery_temp;

    // Battery voltage (mV)
    int battery_volt;

    // Power supplied flag
    bool power_supplied;

    // Callback information
    struct CallbackInfo {
        int cbId;
        ScePowerCallback callback;
        void* userData;
    };

    // Callbacks
    std::vector<CallbackInfo> callbacks;

    // Next callback ID
    int next_callback_id;

    // Battery monitoring thread
    std::unique_ptr<std::thread> battery_thread;
    std::atomic<bool> thread_running;

    // Mutex for thread safety
    mutable std::mutex mutex;

    // Battery monitoring thread function
    void battery_thread_func();

    // Notify all registered callbacks
    void notify_callbacks(int notifyId, int powerInfo);

    // Helper functions
    bool is_valid_cpu_freq(int freq) const;
    bool is_valid_bus_freq(int freq) const;
    bool is_valid_gpu_freq(int freq) const;
    int read_switch_battery_info();
};

// Module functions
int scePowerRegisterCallback(int cbId, ScePowerCallback callback, void* userData);
int scePowerUnregisterCallback(int cbId);
int scePowerGetBatteryLifePercent();
int scePowerGetBatteryLifeTime();
int scePowerGetBatteryTemp();
int scePowerGetBatteryVolt();
int scePowerIsBatteryCharging();
int scePowerIsPowerOnline();
int scePowerGetBatteryStatus();
int scePowerGetCpuClockFrequency();
int scePowerSetCpuClockFrequency(int freq);
int scePowerGetBusClockFrequency();
int scePowerSetBusClockFrequency(int freq);
int scePowerGetGpuClockFrequency();
int scePowerSetGpuClockFrequency(int freq);
int scePowerRequestColdReset();
int scePowerRequestStandby();
int scePowerRequestSuspend();
int scePowerTick(int type);

} // namespace power
} // namespace modules
} // namespace firmware
