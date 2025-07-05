#include "game_profiles.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <json.hpp>

namespace shader {

using json = nlohmann::json;

struct GameProfileManager::GameProfileManagerImpl {
    std::unordered_map<std::string, GameProfile> profiles;
    std::unordered_map<std::string, std::vector<PerformanceMetrics>> metrics_history;
    
    // Constants for optimization
    static constexpr uint32_t MAX_HISTORY_SIZE = 1000;
    static constexpr float PERFORMANCE_THRESHOLD = 0.9f;
    static constexpr float THERMAL_THRESHOLD = 80.0f;
    static constexpr uint32_t MIN_SAMPLES_FOR_ANALYSIS = 100;
};

GameProfileManager::GameProfileManager() : impl(std::make_unique<GameProfileManagerImpl>()) {}

GameProfileManager::~GameProfileManager() = default;

bool GameProfileManager::load_profiles(const std::string& profile_dir) {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(profile_dir)) {
            if (entry.path().extension() == ".json") {
                std::ifstream file(entry.path());
                json j;
                file >> j;
                
                GameProfile profile;
                profile.game_id = j["game_id"];
                profile.game_name = j["game_name"];
                profile.version = j["version"];
                
                // Load performance targets
                profile.target_fps = j["target_fps"];
                profile.min_fps = j["min_fps"];
                profile.target_resolution_width = j["target_resolution_width"];
                profile.target_resolution_height = j["target_resolution_height"];
                
                // Load power and thermal settings
                profile.power_state.gpu_clock_mhz = j["power_state"]["gpu_clock_mhz"];
                profile.power_state.memory_clock_mhz = j["power_state"]["memory_clock_mhz"];
                profile.power_state.voltage_mv = j["power_state"]["voltage_mv"];
                profile.target_temperature = j["target_temperature"];
                profile.aggressive_power_saving = j["aggressive_power_saving"];
                
                // Load optimization profiles
                auto& shader_opts = j["shader_opts"];
                profile.shader_opts = {
                    .use_fast_math = shader_opts["use_fast_math"],
                    .unroll_loops = shader_opts["unroll_loops"],
                    .flatten_control_flow = shader_opts["flatten_control_flow"],
                    .optimize_barriers = shader_opts["optimize_barriers"],
                    .max_registers = shader_opts["max_registers"],
                    .min_blocks_for_loop = shader_opts["min_blocks_for_loop"],
                    .max_unroll_iterations = shader_opts["max_unroll_iterations"],
                    .inline_threshold = shader_opts["inline_threshold"]
                };
                
                auto& memory_opts = j["memory_opts"];
                profile.memory_opts = {
                    .use_texture_compression = memory_opts["use_texture_compression"],
                    .optimize_uniform_layout = memory_opts["optimize_uniform_layout"],
                    .coalesce_memory_access = memory_opts["coalesce_memory_access"],
                    .use_shared_memory = memory_opts["use_shared_memory"],
                    .texture_cache_size_kb = memory_opts["texture_cache_size_kb"],
                    .uniform_buffer_size_kb = memory_opts["uniform_buffer_size_kb"],
                    .shared_memory_size_kb = memory_opts["shared_memory_size_kb"]
                };
                
                auto& post_opts = j["post_opts"];
                profile.post_opts = {
                    .enable_fxaa = post_opts["enable_fxaa"],
                    .enable_color_correction = post_opts["enable_color_correction"],
                    .enable_bloom = post_opts["enable_bloom"],
                    .enable_dof = post_opts["enable_dof"]
                };
                
                // Load quirks
                auto& quirks = j["quirks"];
                profile.quirks = {
                    .needs_vertex_cache_flush = quirks["needs_vertex_cache_flush"],
                    .sensitive_to_precision = quirks["sensitive_to_precision"],
                    .heavy_compute_usage = quirks["heavy_compute_usage"],
                    .texture_heavy = quirks["texture_heavy"],
                    .geometry_heavy = quirks["geometry_heavy"],
                    .problematic_shaders = quirks["problematic_shaders"].get<std::vector<std::string>>()
                };
                
                // Load characteristics
                auto& chars = j["characteristics"];
                profile.characteristics = {
                    .avg_shader_complexity = chars["avg_shader_complexity"],
                    .avg_draw_calls_per_frame = chars["avg_draw_calls_per_frame"],
                    .avg_triangle_count = chars["avg_triangle_count"],
                    .avg_texture_usage_mb = chars["avg_texture_usage_mb"],
                    .avg_compute_usage = chars["avg_compute_usage"]
                };
                
                impl->profiles[profile.game_id] = profile;
            }
        }
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool GameProfileManager::save_profiles(const std::string& profile_dir) {
    try {
        std::filesystem::create_directories(profile_dir);
        
        for (const auto& [game_id, profile] : impl->profiles) {
            json j;
            
            // Basic info
            j["game_id"] = profile.game_id;
            j["game_name"] = profile.game_name;
            j["version"] = profile.version;
            
            // Performance targets
            j["target_fps"] = profile.target_fps;
            j["min_fps"] = profile.min_fps;
            j["target_resolution_width"] = profile.target_resolution_width;
            j["target_resolution_height"] = profile.target_resolution_height;
            
            // Power and thermal settings
            j["power_state"] = {
                {"gpu_clock_mhz", profile.power_state.gpu_clock_mhz},
                {"memory_clock_mhz", profile.power_state.memory_clock_mhz},
                {"voltage_mv", profile.power_state.voltage_mv}
            };
            j["target_temperature"] = profile.target_temperature;
            j["aggressive_power_saving"] = profile.aggressive_power_saving;
            
            // Optimization profiles
            j["shader_opts"] = {
                {"use_fast_math", profile.shader_opts.use_fast_math},
                {"unroll_loops", profile.shader_opts.unroll_loops},
                {"flatten_control_flow", profile.shader_opts.flatten_control_flow},
                {"optimize_barriers", profile.shader_opts.optimize_barriers},
                {"max_registers", profile.shader_opts.max_registers},
                {"min_blocks_for_loop", profile.shader_opts.min_blocks_for_loop},
                {"max_unroll_iterations", profile.shader_opts.max_unroll_iterations},
                {"inline_threshold", profile.shader_opts.inline_threshold}
            };
            
            j["memory_opts"] = {
                {"use_texture_compression", profile.memory_opts.use_texture_compression},
                {"optimize_uniform_layout", profile.memory_opts.optimize_uniform_layout},
                {"coalesce_memory_access", profile.memory_opts.coalesce_memory_access},
                {"use_shared_memory", profile.memory_opts.use_shared_memory},
                {"texture_cache_size_kb", profile.memory_opts.texture_cache_size_kb},
                {"uniform_buffer_size_kb", profile.memory_opts.uniform_buffer_size_kb},
                {"shared_memory_size_kb", profile.memory_opts.shared_memory_size_kb}
            };
            
            j["post_opts"] = {
                {"enable_fxaa", profile.post_opts.enable_fxaa},
                {"enable_color_correction", profile.post_opts.enable_color_correction},
                {"enable_bloom", profile.post_opts.enable_bloom},
                {"enable_dof", profile.post_opts.enable_dof}
            };
            
            // Quirks and characteristics
            j["quirks"] = {
                {"needs_vertex_cache_flush", profile.quirks.needs_vertex_cache_flush},
                {"sensitive_to_precision", profile.quirks.sensitive_to_precision},
                {"heavy_compute_usage", profile.quirks.heavy_compute_usage},
                {"texture_heavy", profile.quirks.texture_heavy},
                {"geometry_heavy", profile.quirks.geometry_heavy},
                {"problematic_shaders", profile.quirks.problematic_shaders}
            };
            
            j["characteristics"] = {
                {"avg_shader_complexity", profile.characteristics.avg_shader_complexity},
                {"avg_draw_calls_per_frame", profile.characteristics.avg_draw_calls_per_frame},
                {"avg_triangle_count", profile.characteristics.avg_triangle_count},
                {"avg_texture_usage_mb", profile.characteristics.avg_texture_usage_mb},
                {"avg_compute_usage", profile.characteristics.avg_compute_usage}
            };
            
            std::ofstream file(profile_dir + "/" + game_id + ".json");
            file << j.dump(4);
        }
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void GameProfileManager::add_profile(const GameProfile& profile) {
    impl->profiles[profile.game_id] = profile;
    validate_profile(impl->profiles[profile.game_id]);
}

void GameProfileManager::remove_profile(const std::string& game_id) {
    impl->profiles.erase(game_id);
    impl->metrics_history.erase(game_id);
}

GameProfile* GameProfileManager::get_profile(const std::string& game_id) {
    auto it = impl->profiles.find(game_id);
    return it != impl->profiles.end() ? &it->second : nullptr;
}

void GameProfileManager::apply_profile(const std::string& game_id,
                                     PerformanceManager& perf_manager,
                                     PostProcessor& post_processor) {
    auto profile = get_profile(game_id);
    if (!profile) return;
    
    // Apply power and thermal settings
    perf_manager.set_power_state(profile->power_state);
    perf_manager.set_thermal_policy(profile->target_temperature,
                                  GameProfileManagerImpl::THERMAL_THRESHOLD);
    
    // Apply resolution scaling
    float scale = static_cast<float>(profile->target_resolution_width) /
                 static_cast<float>(1920); // Assuming 1920x1080 base
    perf_manager.set_resolution_scale_range(scale * 0.75f, scale);
    
    // Apply post-processing settings
    post_processor.enable_effect(PostEffectType::FXAA, profile->post_opts.enable_fxaa);
    post_processor.enable_effect(PostEffectType::ColorCorrection,
                               profile->post_opts.enable_color_correction);
    post_processor.enable_effect(PostEffectType::Bloom, profile->post_opts.enable_bloom);
    post_processor.enable_effect(PostEffectType::DOF, profile->post_opts.enable_dof);
    
    // Set quality based on performance headroom
    if (profile->characteristics.avg_shader_complexity > 0.8f) {
        post_processor.set_quality_preset(PostProcessor::QualityPreset::Low);
    } else if (profile->characteristics.avg_shader_complexity > 0.6f) {
        post_processor.set_quality_preset(PostProcessor::QualityPreset::Medium);
    } else {
        post_processor.set_quality_preset(PostProcessor::QualityPreset::High);
    }
}

void GameProfileManager::analyze_game(const std::string& game_id,
                                    const PerformanceMetrics& metrics,
                                    const ThermalState& thermal_state) {
    auto profile = get_profile(game_id);
    if (!profile) {
        GameProfile new_profile;
        new_profile.game_id = game_id;
        generate_default_profile(new_profile);
        impl->profiles[game_id] = new_profile;
        profile = &impl->profiles[game_id];
    }
    
    // Update metrics history
    auto& history = impl->metrics_history[game_id];
    history.push_back(metrics);
    if (history.size() > GameProfileManagerImpl::MAX_HISTORY_SIZE) {
        history.erase(history.begin());
    }
    
    // Analyze performance patterns
    if (history.size() >= GameProfileManagerImpl::MIN_SAMPLES_FOR_ANALYSIS) {
        analyze_performance_patterns(*profile, history);
    }
    
    // Detect bottlenecks
    detect_bottlenecks(*profile, metrics);
    
    // Update profile if needed
    if (metrics.fps < profile->min_fps ||
        thermal_state.current_temp > profile->target_temperature) {
        optimize_profile(game_id);
    }
}

void GameProfileManager::update_profile_stats(const std::string& game_id,
                                            const PerformanceMetrics& metrics) {
    auto profile = get_profile(game_id);
    if (!profile) return;
    
    // Update running averages
    auto& chars = profile->characteristics;
    const float alpha = 0.05f; // Exponential moving average factor
    
    chars.avg_shader_complexity = (chars.avg_shader_complexity * (1.0f - alpha)) +
                                (metrics.gpu_load_percent / 100.0f * alpha);
    chars.avg_texture_usage_mb = (chars.avg_texture_usage_mb * (1.0f - alpha)) +
                                (metrics.memory_usage_mb * alpha);
    chars.avg_compute_usage = (chars.avg_compute_usage * (1.0f - alpha)) +
                             (metrics.gpu_load_percent / 100.0f * alpha);
}

void GameProfileManager::optimize_profile(const std::string& game_id) {
    auto profile = get_profile(game_id);
    if (!profile) return;
    
    optimize_shader_settings(*profile);
    optimize_memory_settings(*profile);
    optimize_post_processing(*profile);
}

void GameProfileManager::suggest_optimizations(const std::string& game_id,
                                             std::vector<std::string>& suggestions) {
    auto profile = get_profile(game_id);
    if (!profile) return;
    
    identify_optimization_opportunities(*profile, suggestions);
}

// Private methods

void GameProfileManager::validate_profile(GameProfile& profile) {
    // Validate and clamp values to reasonable ranges
    profile.target_fps = std::clamp(profile.target_fps, 30.0f, 60.0f);
    profile.min_fps = std::clamp(profile.min_fps, 15.0f, profile.target_fps);
    profile.target_temperature = std::clamp(profile.target_temperature, 60.0f, 85.0f);
    
    profile.shader_opts.max_registers = std::clamp(profile.shader_opts.max_registers, 32u, 256u);
    profile.shader_opts.max_unroll_iterations = std::clamp(profile.shader_opts.max_unroll_iterations, 1u, 32u);
    profile.shader_opts.inline_threshold = std::clamp(profile.shader_opts.inline_threshold, 1.0f, 10.0f);
    
    profile.memory_opts.texture_cache_size_kb = std::clamp(profile.memory_opts.texture_cache_size_kb, 64u, 8192u);
    profile.memory_opts.uniform_buffer_size_kb = std::clamp(profile.memory_opts.uniform_buffer_size_kb, 16u, 1024u);
    profile.memory_opts.shared_memory_size_kb = std::clamp(profile.memory_opts.shared_memory_size_kb, 16u, 64u);
}

void GameProfileManager::merge_profiles(GameProfile& target, const GameProfile& source) {
    // Merge optimization settings while preserving game-specific quirks
    target.shader_opts = source.shader_opts;
    target.memory_opts = source.memory_opts;
    target.post_opts = source.post_opts;
    
    // Keep the more conservative performance targets
    target.target_fps = std::min(target.target_fps, source.target_fps);
    target.min_fps = std::min(target.min_fps, source.min_fps);
    target.target_temperature = std::min(target.target_temperature, source.target_temperature);
    
    // Merge quirks lists
    target.quirks.problematic_shaders.insert(
        target.quirks.problematic_shaders.end(),
        source.quirks.problematic_shaders.begin(),
        source.quirks.problematic_shaders.end());
}

void GameProfileManager::generate_default_profile(GameProfile& profile) {
    profile.target_fps = 60.0f;
    profile.min_fps = 30.0f;
    profile.target_resolution_width = 1280;
    profile.target_resolution_height = 720;
    profile.target_temperature = 75.0f;
    profile.aggressive_power_saving = false;
    
    // Default shader optimization settings
    profile.shader_opts = {
        .use_fast_math = true,
        .unroll_loops = true,
        .flatten_control_flow = true,
        .optimize_barriers = true,
        .max_registers = 128,
        .min_blocks_for_loop = 4,
        .max_unroll_iterations = 16,
        .inline_threshold = 5.0f
    };
    
    // Default memory optimization settings
    profile.memory_opts = {
        .use_texture_compression = true,
        .optimize_uniform_layout = true,
        .coalesce_memory_access = true,
        .use_shared_memory = true,
        .texture_cache_size_kb = 4096,
        .uniform_buffer_size_kb = 256,
        .shared_memory_size_kb = 32
    };
    
    // Default post-processing settings
    profile.post_opts = {
        .enable_fxaa = true,
        .enable_color_correction = true,
        .enable_bloom = false,
        .enable_dof = false
    };
}

void GameProfileManager::optimize_shader_settings(GameProfile& profile) {
    const auto& chars = profile.characteristics;
    
    // Adjust shader optimization based on complexity
    if (chars.avg_shader_complexity > 0.8f) {
        profile.shader_opts.use_fast_math = true;
        profile.shader_opts.unroll_loops = false;
        profile.shader_opts.max_registers = 96;
        profile.shader_opts.max_unroll_iterations = 8;
    } else if (chars.avg_shader_complexity > 0.6f) {
        profile.shader_opts.use_fast_math = true;
        profile.shader_opts.unroll_loops = true;
        profile.shader_opts.max_registers = 128;
        profile.shader_opts.max_unroll_iterations = 16;
    } else {
        profile.shader_opts.use_fast_math = false;
        profile.shader_opts.unroll_loops = true;
        profile.shader_opts.max_registers = 192;
        profile.shader_opts.max_unroll_iterations = 32;
    }
}

void GameProfileManager::optimize_memory_settings(GameProfile& profile) {
    const auto& chars = profile.characteristics;
    
    // Adjust memory settings based on usage patterns
    if (chars.avg_texture_usage_mb > 1024) {
        profile.memory_opts.use_texture_compression = true;
        profile.memory_opts.texture_cache_size_kb = 8192;
    } else if (chars.avg_texture_usage_mb > 512) {
        profile.memory_opts.use_texture_compression = true;
        profile.memory_opts.texture_cache_size_kb = 4096;
    } else {
        profile.memory_opts.use_texture_compression = false;
        profile.memory_opts.texture_cache_size_kb = 2048;
    }
    
    // Adjust shared memory usage based on compute workload
    if (chars.avg_compute_usage > 0.7f) {
        profile.memory_opts.use_shared_memory = true;
        profile.memory_opts.shared_memory_size_kb = 48;
    } else {
        profile.memory_opts.use_shared_memory = false;
        profile.memory_opts.shared_memory_size_kb = 16;
    }
}

void GameProfileManager::optimize_post_processing(GameProfile& profile) {
    const auto& chars = profile.characteristics;
    
    // Disable expensive post-processing for complex scenes
    if (chars.avg_shader_complexity > 0.8f) {
        profile.post_opts.enable_bloom = false;
        profile.post_opts.enable_dof = false;
    } else if (chars.avg_shader_complexity > 0.6f) {
        profile.post_opts.enable_bloom = true;
        profile.post_opts.enable_dof = false;
    } else {
        profile.post_opts.enable_bloom = true;
        profile.post_opts.enable_dof = true;
    }
    
    // Always keep FXAA for image quality
    profile.post_opts.enable_fxaa = true;
}

void GameProfileManager::analyze_performance_patterns(GameProfile& profile,
                                                    const std::vector<PerformanceMetrics>& history) {
    // Calculate performance stability
    float avg_fps = 0.0f;
    float fps_variance = 0.0f;
    
    for (const auto& metrics : history) {
        avg_fps += metrics.fps;
    }
    avg_fps /= history.size();
    
    for (const auto& metrics : history) {
        float diff = metrics.fps - avg_fps;
        fps_variance += diff * diff;
    }
    fps_variance /= history.size();
    
    // Update profile based on stability
    if (fps_variance > 100.0f) {
        profile.quirks.sensitive_to_precision = true;
        profile.shader_opts.use_fast_math = false;
    }
    
    // Detect compute-heavy workloads
    float avg_compute = 0.0f;
    for (const auto& metrics : history) {
        avg_compute += metrics.gpu_load_percent;
    }
    avg_compute /= history.size();
    
    profile.quirks.heavy_compute_usage = avg_compute > 80.0f;
}

void GameProfileManager::detect_bottlenecks(GameProfile& profile,
                                          const PerformanceMetrics& metrics) {
    // Update characteristics based on current metrics
    profile.characteristics.avg_shader_complexity = metrics.gpu_load_percent / 100.0f;
    profile.characteristics.avg_texture_usage_mb = metrics.memory_usage_mb;
    
    // Detect memory pressure
    if (metrics.memory_usage_mb > 3072) {
        profile.quirks.texture_heavy = true;
    }
    
    // Detect geometry bottlenecks
    if (metrics.gpu_load_percent > 90 && metrics.memory_usage_mb < 1024) {
        profile.quirks.geometry_heavy = true;
    }
}

void GameProfileManager::identify_optimization_opportunities(GameProfile& profile,
                                                          std::vector<std::string>& opportunities) {
    opportunities.clear();
    
    // Shader optimization opportunities
    if (profile.characteristics.avg_shader_complexity > 0.8f) {
        opportunities.push_back("High shader complexity - consider simplifying shaders");
        opportunities.push_back("Enable aggressive shader optimization");
    }
    
    // Memory optimization opportunities
    if (profile.quirks.texture_heavy) {
        opportunities.push_back("High texture memory usage - enable texture compression");
        opportunities.push_back("Optimize texture cache size and management");
    }
    
    // Performance optimization opportunities
    if (profile.quirks.heavy_compute_usage) {
        opportunities.push_back("Heavy compute usage - optimize compute shaders");
        opportunities.push_back("Consider using shared memory for compute");
    }
    
    if (profile.quirks.geometry_heavy) {
        opportunities.push_back("Geometry-bound - optimize mesh complexity");
        opportunities.push_back("Consider using mesh shaders");
    }
    
    // Post-processing optimization opportunities
    if (profile.characteristics.avg_shader_complexity > 0.6f &&
        (profile.post_opts.enable_bloom || profile.post_opts.enable_dof)) {
        opportunities.push_back("Consider disabling expensive post-processing effects");
    }
}

} // namespace shader 