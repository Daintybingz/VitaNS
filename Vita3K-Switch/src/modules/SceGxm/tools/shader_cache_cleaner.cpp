#include "../SceGxm.h"
#include <iostream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

// Helper to convert file_time_type to system_clock::time_point
std::chrono::system_clock::time_point to_sys_time(std::filesystem::file_time_type ft) {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(ft - std::filesystem::file_time_type::clock::now()
        + system_clock::now());
    return sctp;
}

int main() {
    // [Do not instantiate SceGxm directly. Use static/utility functions or provide a minimal mock if needed.]
    
    // Get cache directory
    fs::path cacheDir = fs::current_path() / "shader_cache";
    
    if (!fs::exists(cacheDir)) {
        std::cerr << "Shader cache directory not found" << std::endl;
        return 1;
    }

    // Clean up old shaders
    uint32_t cleaned = 0;
    for (const auto& entry : fs::directory_iterator(cacheDir)) {
        if (entry.is_regular_file()) {
            // Get file modification time
            auto lastWrite = fs::last_write_time(entry);
            auto now = std::chrono::system_clock::now();
            
            // Check if shader is too old
            auto lastWriteSys = to_sys_time(lastWrite);
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastWriteSys).count() > 60 * 60 * 24) {
                fs::remove(entry);
                cleaned++;
            }
        }
    }

    std::cout << "Cleaned up " << cleaned << " old shaders" << std::endl;
    return 0;
}
