#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <filesystem>

namespace shader {

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

    // Initialize the cache with a directory path
    bool initialize(const std::string& cache_dir);

    // Store a shader in the cache
    bool store_shader(uint64_t hash, const std::string& glsl_code, uint32_t gl_program_id);

    // Retrieve a shader from the cache
    bool retrieve_shader(uint64_t hash, std::string& glsl_code, uint32_t& gl_program_id);

    // Save the cache to disk
    bool save_to_disk();

    // Load the cache from disk
    bool load_from_disk();

    // Clear the cache
    void clear();

    // Set the maximum cache size in bytes
    void set_max_size(size_t max_bytes);

    // Get cache statistics
    void get_stats(size_t& total_shaders, size_t& total_size, size_t& hits, size_t& misses);

private:
    // Cache storage
    std::unordered_map<uint64_t, CachedShader> shader_map;

    // Cache directory path
    std::filesystem::path cache_dir;

    // Cache statistics
    struct {
        size_t hits = 0;
        size_t misses = 0;
        size_t total_size_bytes = 0;
        size_t max_size_bytes = 100 * 1024 * 1024; // Default 100MB
    } stats;

    // Thread safety
    std::mutex cache_mutex;

    // Helper functions
    void evict_old_entries();
    std::string get_cache_file_path(uint64_t hash);
    bool serialize_shader(const CachedShader& shader, std::vector<uint8_t>& data);
    bool deserialize_shader(const std::vector<uint8_t>& data, CachedShader& shader);
    void update_shader_timestamp(uint64_t hash);
};

} // namespace shader
