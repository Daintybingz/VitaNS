#include "include/shader_cache.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <algorithm>
#include <functional>
#include <sstream>
#include <cstring>
#include <xxhash.h>

namespace shader {

ShaderCache::ShaderCache() {
    stats.hits = 0;
    stats.misses = 0;
    stats.total_size_bytes = 0;
    stats.max_size_bytes = 100 * 1024 * 1024; // Default 100MB
}

ShaderCache::~ShaderCache() {
    save_to_disk();
}

bool ShaderCache::initialize(const std::string& cache_dir) {
    this->cache_dir = cache_dir;
    
    // Create cache directory if it doesn't exist
    if (!std::filesystem::exists(this->cache_dir)) {
        try {
            std::filesystem::create_directories(this->cache_dir);
        } catch (const std::filesystem::filesystem_error& e) {
            return false;
        }
    }
    
    // Load existing cache
    load_from_disk();
    
    return true;
}

bool ShaderCache::store_shader(uint64_t hash, const std::string& glsl_code, uint32_t gl_program_id) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    
    CachedShader shader{
        .glsl_code = glsl_code,
        .gl_program_id = gl_program_id,
        .last_used = std::chrono::system_clock::now().time_since_epoch().count(),
        .size_bytes = glsl_code.size()
    };
    
    // Check if we need to evict old entries
    if (stats.total_size_bytes + shader.size_bytes > stats.max_size_bytes) {
        evict_old_entries();
    }
    
    shader_map[hash] = shader;
    stats.total_size_bytes += shader.size_bytes;
    
    return true;
}

bool ShaderCache::retrieve_shader(uint64_t hash, std::string& glsl_code, uint32_t& gl_program_id) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    
    auto it = shader_map.find(hash);
    if (it != shader_map.end()) {
        glsl_code = it->second.glsl_code;
        gl_program_id = it->second.gl_program_id;
        update_shader_timestamp(hash);
        stats.hits++;
        return true;
    }
    
    stats.misses++;
    return false;
}

bool ShaderCache::save_to_disk() {
    std::lock_guard<std::mutex> lock(cache_mutex);
    
    try {
        for (const auto& [hash, shader] : shader_map) {
            std::string file_path = get_cache_file_path(hash);
            std::vector<uint8_t> data;
            if (serialize_shader(shader, data)) {
                std::ofstream file(file_path, std::ios::binary);
                if (file.is_open()) {
                    file.write(reinterpret_cast<const char*>(data.data()), data.size());
                }
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool ShaderCache::load_from_disk() {
    std::lock_guard<std::mutex> lock(cache_mutex);
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(cache_dir)) {
            if (entry.path().extension() == ".cache") {
                std::string filename = entry.path().stem().string();
                uint64_t hash = std::stoull(filename);
                
                std::ifstream file(entry.path(), std::ios::binary);
                if (file.is_open()) {
                    std::vector<uint8_t> data(
                        (std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>()
                    );
                    
                    CachedShader shader;
                    if (deserialize_shader(data, shader)) {
                        shader_map[hash] = shader;
                        stats.total_size_bytes += shader.size_bytes;
                    }
                }
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

void ShaderCache::clear() {
    std::lock_guard<std::mutex> lock(cache_mutex);
    shader_map.clear();
    stats.total_size_bytes = 0;
    stats.hits = 0;
    stats.misses = 0;
}

void ShaderCache::set_max_size(size_t max_bytes) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    stats.max_size_bytes = max_bytes;
    evict_old_entries();
}

void ShaderCache::get_stats(size_t& total_shaders, size_t& total_size, size_t& hits, size_t& misses) {
    std::lock_guard<std::mutex> lock(cache_mutex);
    total_shaders = shader_map.size();
    total_size = stats.total_size_bytes;
    hits = stats.hits;
    misses = stats.misses;
}

void ShaderCache::evict_old_entries() {
    if (stats.total_size_bytes <= stats.max_size_bytes) {
        return;
    }
    
    // Convert to vector for sorting
    std::vector<std::pair<uint64_t, CachedShader>> entries(shader_map.begin(), shader_map.end());
    
    // Sort by last used time (oldest first)
    std::sort(entries.begin(), entries.end(),
        [](const auto& a, const auto& b) {
            return a.second.last_used < b.second.last_used;
        });
    
    // Remove oldest entries until we're under the limit
    for (const auto& [hash, shader] : entries) {
        if (stats.total_size_bytes <= stats.max_size_bytes) {
            break;
        }
        
        stats.total_size_bytes -= shader.size_bytes;
        shader_map.erase(hash);
        
        // Remove from disk
        std::filesystem::remove(get_cache_file_path(hash));
    }
}

std::string ShaderCache::get_cache_file_path(uint64_t hash) {
    return (cache_dir / std::to_string(hash)).string() + ".cache";
}

bool ShaderCache::serialize_shader(const CachedShader& shader, std::vector<uint8_t>& data) {
    // Simple serialization: size + data
    size_t total_size = sizeof(uint32_t) + shader.glsl_code.size() + 
                       sizeof(uint32_t) + sizeof(uint64_t) + sizeof(size_t);
    
    data.resize(total_size);
    size_t offset = 0;
    
    // Write glsl_code size and data
    uint32_t code_size = static_cast<uint32_t>(shader.glsl_code.size());
    std::memcpy(data.data() + offset, &code_size, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    std::memcpy(data.data() + offset, shader.glsl_code.data(), shader.glsl_code.size());
    offset += shader.glsl_code.size();
    
    // Write other fields
    std::memcpy(data.data() + offset, &shader.gl_program_id, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    std::memcpy(data.data() + offset, &shader.last_used, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    
    std::memcpy(data.data() + offset, &shader.size_bytes, sizeof(size_t));
    
    return true;
}

bool ShaderCache::deserialize_shader(const std::vector<uint8_t>& data, CachedShader& shader) {
    if (data.size() < sizeof(uint32_t)) {
        return false;
    }
    
    size_t offset = 0;
    
    // Read glsl_code size and data
    uint32_t code_size;
    std::memcpy(&code_size, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    if (offset + code_size + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(size_t) > data.size()) {
        return false;
    }
    
    shader.glsl_code.resize(code_size);
    std::memcpy(&shader.glsl_code[0], data.data() + offset, code_size);
    offset += code_size;
    
    // Read other fields
    std::memcpy(&shader.gl_program_id, data.data() + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    std::memcpy(&shader.last_used, data.data() + offset, sizeof(uint64_t));
    offset += sizeof(uint64_t);
    
    std::memcpy(&shader.size_bytes, data.data() + offset, sizeof(size_t));
    
    return true;
}

void ShaderCache::update_shader_timestamp(uint64_t hash) {
    auto it = shader_map.find(hash);
    if (it != shader_map.end()) {
        it->second.last_used = std::chrono::system_clock::now().time_since_epoch().count();
    }
}

} // namespace shader
