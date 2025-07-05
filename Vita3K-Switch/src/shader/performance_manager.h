#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <memory>

namespace shader {

struct PerformanceMetrics {
    float fps;
    float frame_time_ms;
    float gpu_load_percent;
    float memory_usage_mb;
    float power_usage_mw;
    float temperature_celsius;
};

struct PowerState {
    uint32_t gpu_clock_mhz;
    uint32_t memory_clock_mhz;
    uint32_t voltage_mv;
    bool dynamic_voltage_enabled;
};

struct ThermalState {
    float current_temp;
    float max_temp;
    float target_temp;
    uint32_t fan_speed_percent;
};

class PerformanceManager {
public:
    PerformanceManager();
    ~PerformanceManager();

    // Initialization
    bool initialize();

    // Performance monitoring
    PerformanceMetrics get_current_metrics();
    void update_metrics();
    bool is_thermal_throttling();
    bool is_power_throttling();

    // Power management
    void set_power_state(const PowerState& state);
    PowerState get_current_power_state() const;
    void enable_dynamic_voltage(bool enabled);
    void set_performance_mode(bool high_performance);

    // Thermal management
    void set_thermal_policy(float target_temp, float max_temp);
    void update_thermal_state();
    ThermalState get_thermal_state() const;

    // Resolution scaling
    void enable_dynamic_resolution(bool enabled);
    void set_resolution_scale_range(float min_scale, float max_scale);
    float get_current_resolution_scale() const;
    void update_resolution_scale();

    // Performance optimization
    void optimize_for_battery_life();
    void optimize_for_performance();
    void optimize_for_thermal();
    void apply_game_specific_optimizations(const std::string& game_id);

    // Statistics and monitoring
    struct PerformanceStats {
        float average_fps;
        float min_fps;
        float max_fps;
        float average_frame_time;
        float average_gpu_load;
        float average_power_usage;
        float total_runtime_hours;
    };

    PerformanceStats get_stats() const;
    void reset_stats();
    void start_monitoring();
    void stop_monitoring();

private:
    // Internal state
    PerformanceMetrics current_metrics;
    PowerState current_power_state;
    ThermalState current_thermal_state;
    PerformanceStats stats;

    // Configuration
    struct Config {
        bool dynamic_resolution_enabled;
        float min_resolution_scale;
        float max_resolution_scale;
        float current_resolution_scale;
        bool high_performance_mode;
        bool dynamic_voltage_enabled;
        float target_fps;
    } config;

    // Monitoring
    bool monitoring_enabled;
    std::chrono::steady_clock::time_point last_update;
    std::vector<PerformanceMetrics> metrics_history;

    // Internal helpers
    void update_power_state();
    void update_thermal_policy();
    void update_clock_speeds();
    void update_voltage();
    float calculate_optimal_resolution_scale();
    void log_performance_event(const std::string& event);

    // Optimization helpers
    void optimize_shader_parameters();
    void optimize_memory_timings();
    void optimize_thermal_policy();
    void optimize_power_consumption();

    // Game-specific optimizations
    struct GameProfile {
        std::string game_id;
        PowerState recommended_power_state;
        float recommended_resolution_scale;
        bool use_aggressive_power_saving;
    };

    std::vector<GameProfile> game_profiles;
    void load_game_profiles();
    void apply_game_profile(const GameProfile& profile);
};

} // namespace shader 