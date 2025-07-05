#include "include/framebuffer_manager.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <chrono>

namespace renderer::gl {

// Memory optimization function to manage VRAM usage efficiently
void FramebufferManager::optimize_memory_usage() {
    std::lock_guard<std::mutex> lock(fb_mutex);
    
    // Track total memory usage
    size_t total_memory = 0;
    size_t color_memory = 0;
    size_t depth_memory = 0;
    size_t msaa_memory = 0;
    size_t effect_memory = 0;
    
    // Calculate memory usage for all framebuffers
    for (auto& pair : framebuffers) {
        Framebuffer& fb = pair.second;
        
        // Calculate memory usage for this framebuffer
        size_t fb_color_memory = fb.width * fb.height * 4; // RGBA = 4 bytes per pixel
        size_t fb_depth_memory = (fb.depth_stencil_texture != 0) ? fb.width * fb.height * 4 : 0;
        size_t fb_msaa_memory = (fb.is_multisampled) ? (fb_color_memory + fb_depth_memory) * 4 : 0; // Assuming 4x MSAA
        size_t fb_effect_memory = (fb.effect_texture != 0) ? fb.scaled_width * fb.scaled_height * 4 : 0;
        
        size_t fb_total_memory = fb_color_memory + fb_depth_memory + fb_msaa_memory + fb_effect_memory;
        
        // Add to totals
        color_memory += fb_color_memory;
        depth_memory += fb_depth_memory;
        msaa_memory += fb_msaa_memory;
        effect_memory += fb_effect_memory;
        total_memory += fb_total_memory;
    }
    
    // Log memory usage for debugging
    std::cout << "[Renderer] Framebuffer memory usage:" << std::endl;
    std::cout << "  Color buffers: " << (color_memory / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  Depth buffers: " << (depth_memory / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  MSAA buffers: " << (msaa_memory / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  Effect buffers: " << (effect_memory / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "  Total: " << (total_memory / (1024 * 1024)) << " MB" << std::endl;
    
    // Define memory thresholds for the Nintendo Switch (conservative estimates)
    // Switch has about 4GB of RAM, with around 3GB available for applications
    // We'll set a conservative VRAM budget for framebuffers
    const size_t memory_warning = 256 * 1024 * 1024;  // 256 MB - warning threshold
    const size_t memory_critical = 512 * 1024 * 1024; // 512 MB - critical threshold
    
    // If memory usage is above warning threshold, log a warning
    if (total_memory > memory_warning) {
        std::cout << "[Renderer] WARNING: High framebuffer memory usage detected" << std::endl;
    }
    
    // If memory usage is above critical threshold, implement memory-saving strategies
    if (total_memory > memory_critical) {
        std::cout << "[Renderer] CRITICAL: Excessive framebuffer memory usage, applying optimization strategies" << std::endl;
        
        // Strategy 1: Identify and optimize framebuffers by usage pattern
        optimize_by_usage_pattern();
        
        // Strategy 2: Reduce MSAA level for less important framebuffers
        reduce_msaa_level();
        
        // Strategy 3: Share depth buffers where possible
        share_depth_buffers();
        
        // Strategy 4: Reduce resolution of off-screen framebuffers
        reduce_offscreen_resolution();
    }
}

void FramebufferManager::optimize_by_usage_pattern() {
    std::lock_guard<std::mutex> lock(fb_mutex);
    
    // Analyze usage patterns and optimize accordingly
    for (auto& pair : framebuffers) {
        uint32_t handle = pair.first;
        Framebuffer& fb = pair.second;
        
        // If framebuffer is rarely used, reduce its quality
        if (fb.width > 1024 || fb.height > 1024) {
            // Reduce resolution for large, rarely-used framebuffers
            // This would require more complex logic to implement
        }
        
        // Remove MSAA calls - not supported on Switch
        // disable_msaa(handle);
    }
}

void FramebufferManager::reduce_msaa_level() {
    std::lock_guard<std::mutex> lock(fb_mutex);
    
    // Sort framebuffers by size (largest first)
    std::vector<std::pair<uint32_t, uint32_t>> fb_sizes;
    for (auto& pair : framebuffers) {
        uint32_t area = pair.second.width * pair.second.height;
        fb_sizes.push_back({pair.first, area});
    }
    
    std::sort(fb_sizes.begin(), fb_sizes.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Disable MSAA for the largest framebuffers
    for (size_t i = 0; i < std::min(fb_sizes.size(), size_t(3)); ++i) {
        // Remove MSAA calls - not supported on Switch
        // disable_msaa(fb_sizes[i].first);
    }
}

void FramebufferManager::share_depth_buffers() {
    // Group framebuffers by size
    std::unordered_map<uint64_t, std::vector<uint32_t>> size_groups;
    
    for (auto& pair : framebuffers) {
        uint32_t handle = pair.first;
        Framebuffer& fb = pair.second;
        
        if (fb.depth_stencil_texture != 0) {
            // Create a key from width and height
            uint64_t size_key = (static_cast<uint64_t>(fb.width) << 32) | fb.height;
            size_groups[size_key].push_back(handle);
        }
    }
    
    // For each group with multiple framebuffers, share depth buffers
    for (auto& group : size_groups) {
        if (group.second.size() > 1) {
            // Use the first framebuffer's depth buffer as the shared one
            uint32_t primary_handle = group.second[0];
            Framebuffer& primary_fb = framebuffers[primary_handle];
            GLuint shared_depth = primary_fb.depth_stencil_texture;
            
            // Share with other framebuffers in the group
            for (size_t i = 1; i < group.second.size(); i++) {
                uint32_t handle = group.second[i];
                Framebuffer& fb = framebuffers[handle];
                
                // Delete the existing depth buffer
                if (fb.depth_stencil_texture != 0 && fb.depth_stencil_texture != shared_depth) {
                    glDeleteTextures(1, &fb.depth_stencil_texture);
                }
                
                // Use the shared depth buffer
                fb.depth_stencil_texture = shared_depth;
                
                // Update the framebuffer attachment
                glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                      GL_TEXTURE_2D, shared_depth, 0);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 
                                      GL_TEXTURE_2D, shared_depth, 0);
            }
            
            std::cout << "[Renderer] Shared depth buffer among " << group.second.size() 
                      << " framebuffers of size " << primary_fb.width << "x" << primary_fb.height << std::endl;
        }
    }
}

void FramebufferManager::reduce_offscreen_resolution() {
    // Identify offscreen framebuffers (those not matching default size)
    std::vector<uint32_t> offscreen_handles;
    
    for (auto& pair : framebuffers) {
        uint32_t handle = pair.first;
        Framebuffer& fb = pair.second;
        
        // Skip default-sized framebuffers (likely main display)
        if (fb.width == default_width && fb.height == default_height) {
            continue;
        }
        
        // Skip small framebuffers
        if (fb.width < 256 || fb.height < 256) {
            continue;
        }
        
        offscreen_handles.push_back(handle);
    }
    
    // Reduce resolution of offscreen framebuffers
    for (uint32_t handle : offscreen_handles) {
        Framebuffer& fb = framebuffers[handle];
        
        // Calculate new size (75% of original)
        uint32_t new_width = fb.width * 3 / 4;
        uint32_t new_height = fb.height * 3 / 4;
        
        // Skip if already at reduced resolution
        if (fb.width == new_width && fb.height == new_height) {
            continue;
        }
        
        // Create new textures at reduced size
        GLuint new_color_texture;
        glGenTextures(1, &new_color_texture);
        glBindTexture(GL_TEXTURE_2D, new_color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, new_width, new_height, 0, 
                    GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Update framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, new_color_texture, 0);
        
        // Delete old texture
        glDeleteTextures(1, &fb.color_texture);
        
        // Update framebuffer info
        fb.color_texture = new_color_texture;
        fb.width = new_width;
        fb.height = new_height;
        fb.scaled_width = new_width;
        fb.scaled_height = new_height;
        
        std::cout << "[Renderer] Reduced resolution of offscreen framebuffer " << handle 
                  << " to " << new_width << "x" << new_height << std::endl;
    }
}

// Memory usage tracking
void FramebufferManager::log_memory_usage() {
    size_t total_memory = 0;
    
    for (auto& pair : framebuffers) {
        Framebuffer& fb = pair.second;
        
        // Calculate memory usage for this framebuffer
        size_t fb_color_memory = fb.width * fb.height * 4;
        size_t fb_depth_memory = (fb.depth_stencil_texture != 0) ? fb.width * fb.height * 4 : 0;
        size_t fb_msaa_memory = (fb.is_multisampled) ? (fb_color_memory + fb_depth_memory) * 4 : 0;
        size_t fb_effect_memory = (fb.effect_texture != 0) ? fb.scaled_width * fb.scaled_height * 4 : 0;
        
        size_t fb_total_memory = fb_color_memory + fb_depth_memory + fb_msaa_memory + fb_effect_memory;
        total_memory += fb_total_memory;
        
        std::cout << "[Renderer] Framebuffer " << pair.first << ": " 
                  << fb.width << "x" << fb.height 
                  << " (" << (fb_total_memory / 1024) << " KB)" << std::endl;
    }
    
    std::cout << "[Renderer] Total framebuffer memory: " << (total_memory / (1024 * 1024)) << " MB" << std::endl;
}

// Update framebuffer usage tracking
void FramebufferManager::update_usage_tracking(uint32_t handle) {
    static std::unordered_map<uint32_t, std::chrono::steady_clock::time_point> last_usage_time;
    last_usage_time[handle] = std::chrono::steady_clock::now();
}

} // namespace renderer::gl
