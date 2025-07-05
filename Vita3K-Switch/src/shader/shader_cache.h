#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <filesystem>
#include <memory>
#include <chrono>

namespace shader {

struct ShaderVariant {
    std::string glsl_code;
    uint64_t hash;
    ShaderType type;
    std::vector<uint32_t> specialization_constants;
    std::chrono::system_clock::time_point last_used;
    uint32_t use_count;
};

struct PipelineState {
    std::vector<uint32_t> vertex_attributes;
    std::vector<uint32_t> fragment_outputs;
    std::vector<uint32_t> blend_states;
    std::vector<uint32_t> depth_stencil_state;
    uint64_t hash;
};

struct CachedShader {
    std::string glsl_code;
    uint32_t gl_program_id;
    uint64_t last_used;
    size_t size_bytes;
};

class ShaderCache {
public:
    ShaderCache();
    ~ShaderCache();

    // Initialize the cache system
    bool initialize(const std::string& cache_dir);

    // Shader variant management
    uint64_t store_shader_variant(const std::string& glsl_code, 
                                ShaderType type,
                                const std::vector<uint32_t>& specialization_constants);
    
    ShaderVariant* get_shader_variant(uint64_t hash);
    bool has_shader_variant(uint64_t hash);
    void update_shader_variant_usage(uint64_t hash);
    void remove_shader_variant(uint64_t hash);

    // Pipeline state management
    uint64_t store_pipeline_state(const PipelineState& state);
    PipelineState* get_pipeline_state(uint64_t hash);
    bool has_pipeline_state(uint64_t hash);

    // Cache management
    void save_cache_to_disk();
    void load_cache_from_disk();
    void clear_cache();
    void prune_unused_variants(uint32_t max_age_hours = 24);

    // Hot reload support
    void enable_hot_reload(bool enabled);
    void check_for_modifications();
    bool reload_modified_shaders();

    // Statistics
    struct CacheStats {
        uint32_t total_variants;
        uint32_t total_pipeline_states;
        uint32_t cache_hits;
        uint32_t cache_misses;
        uint64_t total_cache_size;
        float hit_rate;
    };

    CacheStats get_stats() const;

private:
    // Internal cache storage
    std::unordered_map<uint64_t, ShaderVariant> shader_variants;
    std::unordered_map<uint64_t, PipelineState> pipeline_states;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> file_timestamps;

    // Cache configuration
    std::string cache_directory;
    bool hot_reload_enabled;
    CacheStats stats;

    // Internal helpers
    uint64_t compute_shader_hash(const std::string& glsl_code, 
                               ShaderType type,
                               const std::vector<uint32_t>& specialization_constants);
    
    uint64_t compute_pipeline_hash(const PipelineState& state);
    void update_stats(bool cache_hit);
    bool save_variant_to_disk(const ShaderVariant& variant);
    bool load_variant_from_disk(uint64_t hash);
    void monitor_shader_files();
    
    // File management
    std::string get_variant_path(uint64_t hash) const;
    std::string get_pipeline_path(uint64_t hash) const;
    bool write_binary_file(const std::string& path, const void* data, size_t size);
    bool read_binary_file(const std::string& path, std::vector<uint8_t>& data);
};

} // namespace shader 