#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "performance_manager.h"
#include "post_processor.h"

namespace shader {

struct ShaderOptimizationProfile {
    bool use_fast_math;
    bool unroll_loops;
    bool flatten_control_flow;
    bool optimize_barriers;
    uint32_t max_registers;
    uint32_t min_blocks_for_loop;
    uint32_t max_unroll_iterations;
    float inline_threshold;
};

struct MemoryOptimizationProfile {
    bool use_texture_compression;
    bool optimize_uniform_layout;
    bool coalesce_memory_access;
    bool use_shared_memory;
    uint32_t texture_cache_size_kb;
    uint32_t uniform_buffer_size_kb;
    uint32_t shared_memory_size_kb;
};

struct PostProcessingProfile {
    bool enable_fxaa;
    bool enable_color_correction;
    bool enable_bloom;
    bool enable_dof;
    PostEffectParams effect_params;
};

struct GameProfile {
    std::string game_id;
    std::string game_name;
    std::string version;
    
    // Performance targets
    float target_fps;
    float min_fps;
    uint32_t target_resolution_width;
    uint32_t target_resolution_height;
    
    // Power and thermal settings
    PowerState power_state;
    float target_temperature;
    bool aggressive_power_saving;
    
    // Optimization profiles
    ShaderOptimizationProfile shader_opts;
    MemoryOptimizationProfile memory_opts;
    PostProcessingProfile post_opts;
    
    // Game-specific quirks
    struct Quirks {
        bool needs_vertex_cache_flush;
        bool sensitive_to_precision;
        bool heavy_compute_usage;
        bool texture_heavy;
        bool geometry_heavy;
        std::vector<std::string> problematic_shaders;
    } quirks;
    
    // Performance characteristics
    struct Characteristics {
        float avg_shader_complexity;
        float avg_draw_calls_per_frame;
        float avg_triangle_count;
        float avg_texture_usage_mb;
        float avg_compute_usage;
    } characteristics;
};

class GameProfileManager {
public:
    GameProfileManager();
    ~GameProfileManager();
    
    // Profile management
    bool load_profiles(const std::string& profile_dir);
    bool save_profiles(const std::string& profile_dir);
    void add_profile(const GameProfile& profile);
    void remove_profile(const std::string& game_id);
    GameProfile* get_profile(const std::string& game_id);
    
    // Profile application
    void apply_profile(const std::string& game_id,
                      class PerformanceManager& perf_manager,
                      class PostProcessor& post_processor);
    
    // Profile generation
    void analyze_game(const std::string& game_id,
                     const PerformanceMetrics& metrics,
                     const ThermalState& thermal_state);
    void update_profile_stats(const std::string& game_id,
                            const PerformanceMetrics& metrics);
    
    // Profile optimization
    void optimize_profile(const std::string& game_id);
    void suggest_optimizations(const std::string& game_id,
                             std::vector<std::string>& suggestions);

private:
    struct GameProfileManagerImpl;
    std::unique_ptr<GameProfileManagerImpl> impl;
    
    // Internal helpers
    void validate_profile(GameProfile& profile);
    void merge_profiles(GameProfile& target, const GameProfile& source);
    void generate_default_profile(GameProfile& profile);
    void optimize_shader_settings(GameProfile& profile);
    void optimize_memory_settings(GameProfile& profile);
    void optimize_post_processing(GameProfile& profile);
    
    // Profile analysis
    void analyze_performance_patterns(GameProfile& profile,
                                    const std::vector<PerformanceMetrics>& history);
    void detect_bottlenecks(GameProfile& profile,
                           const PerformanceMetrics& metrics);
    void identify_optimization_opportunities(GameProfile& profile,
                                          std::vector<std::string>& opportunities);
};

} // namespace shader 