#include "performance_manager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <thread>

namespace shader {

// Tegra X1 specific constants
constexpr uint32_t TEGRA_X1_MAX_GPU_CLOCK = 921;  // MHz
constexpr uint32_t TEGRA_X1_MIN_GPU_CLOCK = 307;  // MHz
constexpr uint32_t TEGRA_X1_MAX_MEM_CLOCK = 1600; // MHz
constexpr uint32_t TEGRA_X1_MIN_MEM_CLOCK = 800;  // MHz
constexpr float TEGRA_X1_MAX_TEMP = 83.0f;        // Celsius
constexpr float TEGRA_X1_TARGET_TEMP = 75.0f;     // Celsius

PerformanceManager::PerformanceManager()
    : monitoring_enabled(false) {
    // Initialize default configuration
    config = {
        .dynamic_resolution_enabled = true,
        .min_resolution_scale = 0.5f,
        .max_resolution_scale = 1.0f,
        .current_resolution_scale = 1.0f,
        .high_performance_mode = false,
        .dynamic_voltage_enabled = true,
        .target_fps = 60.0f
    };

    // Initialize default power state
    current_power_state = {
        .gpu_clock_mhz = TEGRA_X1_MAX_GPU_CLOCK,
        .memory_clock_mhz = TEGRA_X1_MAX_MEM_CLOCK,
        .voltage_mv = 1000,
        .dynamic_voltage_enabled = true
    };

    // Initialize default thermal state
    current_thermal_state = {
        .current_temp = 0.0f,
        .max_temp = TEGRA_X1_MAX_TEMP,
        .target_temp = TEGRA_X1_TARGET_TEMP,
        .fan_speed_percent = 0
    };

    // Initialize performance stats
    reset_stats();
}

PerformanceManager::~PerformanceManager() {
    if (monitoring_enabled) {
        stop_monitoring();
    }
}

bool PerformanceManager::initialize() {
    // Load game profiles
    load_game_profiles();

    // Initialize hardware monitoring
    update_metrics();
    update_thermal_state();
    update_power_state();

    return true;
}

PerformanceMetrics PerformanceManager::get_current_metrics() {
    return current_metrics;
}

void PerformanceManager::update_metrics() {
    auto now = std::chrono::steady_clock::now();
    float delta_time = std::chrono::duration<float>(now - last_update).count();
    last_update = now;

    // Update FPS and frame time
    current_metrics.frame_time_ms = delta_time * 1000.0f;
    current_metrics.fps = 1.0f / delta_time;

    // Update GPU metrics (simulated for now)
    current_metrics.gpu_load_percent = 80.0f; // Example value
    current_metrics.memory_usage_mb = 2048.0f; // Example value
    current_metrics.power_usage_mw = 3000.0f; // Example value
    current_metrics.temperature_celsius = 70.0f; // Example value

    // Update statistics
    if (monitoring_enabled) {
        stats.average_fps = (stats.average_fps * 0.95f) + (current_metrics.fps * 0.05f);
        stats.min_fps = std::min(stats.min_fps, current_metrics.fps);
        stats.max_fps = std::max(stats.max_fps, current_metrics.fps);
        stats.average_frame_time = (stats.average_frame_time * 0.95f) + (current_metrics.frame_time_ms * 0.05f);
        stats.average_gpu_load = (stats.average_gpu_load * 0.95f) + (current_metrics.gpu_load_percent * 0.05f);
        stats.average_power_usage = (stats.average_power_usage * 0.95f) + (current_metrics.power_usage_mw * 0.05f);

        metrics_history.push_back(current_metrics);
        if (metrics_history.size() > 1000) {
            metrics_history.erase(metrics_history.begin());
        }
    }
}

bool PerformanceManager::is_thermal_throttling() {
    return current_metrics.temperature_celsius > current_thermal_state.target_temp;
}

bool PerformanceManager::is_power_throttling() {
    return current_metrics.power_usage_mw > 4000.0f; // Example threshold
}

void PerformanceManager::set_power_state(const PowerState& state) {
    current_power_state = state;
    update_clock_speeds();
    update_voltage();
}

PowerState PerformanceManager::get_current_power_state() const {
    return current_power_state;
}

void PerformanceManager::enable_dynamic_voltage(bool enabled) {
    current_power_state.dynamic_voltage_enabled = enabled;
    if (enabled) {
        optimize_power_consumption();
    }
}

void PerformanceManager::set_performance_mode(bool high_performance) {
    config.high_performance_mode = high_performance;
    if (high_performance) {
        optimize_for_performance();
    } else {
        optimize_for_battery_life();
    }
}

void PerformanceManager::set_thermal_policy(float target_temp, float max_temp) {
    current_thermal_state.target_temp = std::min(target_temp, TEGRA_X1_MAX_TEMP);
    current_thermal_state.max_temp = std::min(max_temp, TEGRA_X1_MAX_TEMP);
    update_thermal_policy();
}

void PerformanceManager::update_thermal_state() {
    // Update thermal metrics
    float temp_delta = current_metrics.temperature_celsius - current_thermal_state.target_temp;
    
    // Adjust fan speed
    if (temp_delta > 0) {
        current_thermal_state.fan_speed_percent = std::min(100u, 
            static_cast<uint32_t>(50 + (temp_delta * 5)));
    } else {
        current_thermal_state.fan_speed_percent = std::max(30u,
            static_cast<uint32_t>(50 + (temp_delta * 5)));
    }

    // Apply thermal throttling if needed
    if (is_thermal_throttling()) {
        optimize_for_thermal();
    }
}

ThermalState PerformanceManager::get_thermal_state() const {
    return current_thermal_state;
}

void PerformanceManager::enable_dynamic_resolution(bool enabled) {
    config.dynamic_resolution_enabled = enabled;
    if (enabled) {
        update_resolution_scale();
    }
}

void PerformanceManager::set_resolution_scale_range(float min_scale, float max_scale) {
    config.min_resolution_scale = std::clamp(min_scale, 0.25f, 1.0f);
    config.max_resolution_scale = std::clamp(max_scale, min_scale, 1.0f);
    update_resolution_scale();
}

float PerformanceManager::get_current_resolution_scale() const {
    return config.current_resolution_scale;
}

void PerformanceManager::update_resolution_scale() {
    if (!config.dynamic_resolution_enabled) return;

    float target_frame_time = 1000.0f / config.target_fps;
    float current_frame_time = current_metrics.frame_time_ms;
    float scale_factor = target_frame_time / current_frame_time;

    // Adjust resolution scale
    float new_scale = config.current_resolution_scale * std::sqrt(scale_factor);
    new_scale = std::clamp(new_scale, config.min_resolution_scale, config.max_resolution_scale);

    // Apply smoothing
    config.current_resolution_scale = config.current_resolution_scale * 0.9f + new_scale * 0.1f;
}

void PerformanceManager::optimize_for_battery_life() {
    PowerState power_state = {
        .gpu_clock_mhz = TEGRA_X1_MIN_GPU_CLOCK,
        .memory_clock_mhz = TEGRA_X1_MIN_MEM_CLOCK,
        .voltage_mv = 800,
        .dynamic_voltage_enabled = true
    };

    set_power_state(power_state);
    enable_dynamic_resolution(true);
    set_resolution_scale_range(0.5f, 0.75f);
    optimize_shader_parameters();
}

void PerformanceManager::optimize_for_performance() {
    PowerState power_state = {
        .gpu_clock_mhz = TEGRA_X1_MAX_GPU_CLOCK,
        .memory_clock_mhz = TEGRA_X1_MAX_MEM_CLOCK,
        .voltage_mv = 1000,
        .dynamic_voltage_enabled = false
    };

    set_power_state(power_state);
    enable_dynamic_resolution(false);
    set_resolution_scale_range(1.0f, 1.0f);
    optimize_shader_parameters();
}

void PerformanceManager::optimize_for_thermal() {
    float temp_delta = current_metrics.temperature_celsius - current_thermal_state.target_temp;
    float throttle_factor = std::clamp(1.0f - (temp_delta / 10.0f), 0.5f, 1.0f);

    PowerState power_state = {
        .gpu_clock_mhz = static_cast<uint32_t>(TEGRA_X1_MAX_GPU_CLOCK * throttle_factor),
        .memory_clock_mhz = static_cast<uint32_t>(TEGRA_X1_MAX_MEM_CLOCK * throttle_factor),
        .voltage_mv = static_cast<uint32_t>(1000 * throttle_factor),
        .dynamic_voltage_enabled = true
    };

    set_power_state(power_state);
    set_resolution_scale_range(0.5f, throttle_factor);
    optimize_thermal_policy();
}

void PerformanceManager::apply_game_specific_optimizations(const std::string& game_id) {
    auto it = std::find_if(game_profiles.begin(), game_profiles.end(),
        [&game_id](const GameProfile& profile) { return profile.game_id == game_id; });

    if (it != game_profiles.end()) {
        apply_game_profile(*it);
    }
}

PerformanceManager::PerformanceStats PerformanceManager::get_stats() const {
    return stats;
}

void PerformanceManager::reset_stats() {
    stats = {
        .average_fps = 0.0f,
        .min_fps = std::numeric_limits<float>::max(),
        .max_fps = 0.0f,
        .average_frame_time = 0.0f,
        .average_gpu_load = 0.0f,
        .average_power_usage = 0.0f,
        .total_runtime_hours = 0.0f
    };
}

void PerformanceManager::start_monitoring() {
    monitoring_enabled = true;
    last_update = std::chrono::steady_clock::now();
    metrics_history.clear();
}

void PerformanceManager::stop_monitoring() {
    monitoring_enabled = false;
}

// Private helper methods

void PerformanceManager::update_power_state() {
    if (current_power_state.dynamic_voltage_enabled) {
        optimize_power_consumption();
    }
    update_clock_speeds();
    update_voltage();
}

void PerformanceManager::update_thermal_policy() {
    float temp_delta = current_metrics.temperature_celsius - current_thermal_state.target_temp;
    
    if (temp_delta > 0) {
        // Temperature is too high, apply thermal throttling
        float throttle_factor = 1.0f - std::min(temp_delta / 10.0f, 0.5f);
        current_power_state.gpu_clock_mhz = static_cast<uint32_t>(current_power_state.gpu_clock_mhz * throttle_factor);
        current_power_state.memory_clock_mhz = static_cast<uint32_t>(current_power_state.memory_clock_mhz * throttle_factor);
        update_clock_speeds();
    }
}

void PerformanceManager::update_clock_speeds() {
    // Clamp clock speeds to valid ranges
    current_power_state.gpu_clock_mhz = std::clamp(current_power_state.gpu_clock_mhz,
        TEGRA_X1_MIN_GPU_CLOCK, TEGRA_X1_MAX_GPU_CLOCK);
    current_power_state.memory_clock_mhz = std::clamp(current_power_state.memory_clock_mhz,
        TEGRA_X1_MIN_MEM_CLOCK, TEGRA_X1_MAX_MEM_CLOCK);
}

void PerformanceManager::update_voltage() {
    if (current_power_state.dynamic_voltage_enabled) {
        // Simple voltage scaling based on GPU clock
        float clock_scale = static_cast<float>(current_power_state.gpu_clock_mhz) / TEGRA_X1_MAX_GPU_CLOCK;
        current_power_state.voltage_mv = static_cast<uint32_t>(800 + (200 * clock_scale));
    }
}

float PerformanceManager::calculate_optimal_resolution_scale() {
    if (!config.dynamic_resolution_enabled) return 1.0f;

    float performance_headroom = config.target_fps / current_metrics.fps;
    float optimal_scale = std::sqrt(1.0f / performance_headroom);
    return std::clamp(optimal_scale, config.min_resolution_scale, config.max_resolution_scale);
}

void PerformanceManager::log_performance_event(const std::string& event) {
    // TODO: Implement performance logging
}

void PerformanceManager::optimize_shader_parameters() {
    // TODO: Implement shader parameter optimization
}

void PerformanceManager::optimize_memory_timings() {
    // TODO: Implement memory timing optimization
}

void PerformanceManager::optimize_thermal_policy() {
    // TODO: Implement thermal policy optimization
}

void PerformanceManager::optimize_power_consumption() {
    // TODO: Implement power consumption optimization
}

void PerformanceManager::load_game_profiles() {
    // TODO: Load game profiles from configuration file
}

void PerformanceManager::apply_game_profile(const GameProfile& profile) {
    set_power_state(profile.recommended_power_state);
    config.current_resolution_scale = profile.recommended_resolution_scale;
    
    if (profile.use_aggressive_power_saving) {
        optimize_for_battery_life();
    }
}

} // namespace shader 