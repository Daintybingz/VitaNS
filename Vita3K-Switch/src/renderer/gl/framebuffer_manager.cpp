#include "include/framebuffer_manager.h"
#include <iostream>
#include <algorithm>

namespace renderer::gl {

FramebufferManager::FramebufferManager()
    : next_handle(1)
    , default_width(960)
    , default_height(544)
    , global_scaling_mode(ScalingMode::Bilinear)
    , global_scaling_factor(1.0f)
    , effects_enabled(true)
    , quad_vbo(0) {
}

FramebufferManager::~FramebufferManager() {
    clear_all();
}

bool FramebufferManager::initialize() {
    std::cout << "[Renderer] Initializing framebuffer manager" << std::endl;
    
    // Create a full-screen quad for post-processing
    glGenBuffers(1, &quad_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    
    // Quad vertices: position (x,y) and texture coordinates (s,t)
    float quad_vertices[] = {
        // Position    // TexCoords
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    return true;
}

uint32_t FramebufferManager::create_framebuffer(const FramebufferParams& params) {
    std::lock_guard<std::mutex> lock(fb_mutex);
    
    Framebuffer fb = {};
    fb.width = params.width;
    fb.height = params.height;
    fb.scaled_width = params.width;
    fb.scaled_height = params.height;
    fb.scaling = params.scaling;
    fb.effect = params.effect;
    fb.is_multisampled = false;
    fb.fbo = 0;
    fb.color_texture = 0;
    fb.depth_stencil_texture = 0;
    fb.effect_fbo = 0;
    fb.effect_texture = 0;
    
    if (!create_framebuffer_internal(fb, params.width, params.height, params.format, 
                                    params.has_depth, params.has_stencil)) {
        return 0;
    }
    
    // Apply scaling if needed
    if (params.scaling != ScalingMode::None && params.scaling_factor != 1.0f) {
        uint32_t scaled_width = static_cast<uint32_t>(params.width * params.scaling_factor);
        uint32_t scaled_height = static_cast<uint32_t>(params.height * params.scaling_factor);
        scale_framebuffer(next_handle, scaled_width, scaled_height);
    }
    
    uint32_t handle = next_handle++;
    framebuffers[handle] = fb;
    
    return handle;
}

bool FramebufferManager::create_framebuffer_internal(Framebuffer& fb, uint32_t width, uint32_t height, 
                                                   uint32_t format, bool has_depth, bool has_stencil) {
    // Generate framebuffer object
    glGenFramebuffers(1, &fb.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
    
    // Create color texture
    glGenTextures(1, &fb.color_texture);
    glBindTexture(GL_TEXTURE_2D, fb.color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.color_texture, 0);
    
    // Create depth/stencil texture if needed
    if (has_depth || has_stencil) {
        glGenTextures(1, &fb.depth_stencil_texture);
        glBindTexture(GL_TEXTURE_2D, fb.depth_stencil_texture);
        
        if (has_depth && has_stencil) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, 
                        GL_DEPTH_ATTACHMENT, GL_UNSIGNED_INT_24_8_OES, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                  GL_TEXTURE_2D, fb.depth_stencil_texture, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, 
                                  GL_TEXTURE_2D, fb.depth_stencil_texture, 0);
        } else if (has_depth) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, 
                        GL_DEPTH_ATTACHMENT, GL_UNSIGNED_INT_24_8_OES, nullptr);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                  GL_TEXTURE_2D, fb.depth_stencil_texture, 0);
        }
    }
    
    // Check framebuffer status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[Renderer] Framebuffer creation failed: " << status << std::endl;
        return false;
    }
    
    return true;
}

bool FramebufferManager::bind_framebuffer(uint32_t handle) {
    std::lock_guard<std::mutex> lock(fb_mutex);
    
    auto it = framebuffers.find(handle);
    if (it == framebuffers.end()) {
        return false;
    }
    
    Framebuffer& fb = it->second;
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
    return true;
}

GLuint FramebufferManager::get_color_texture(uint32_t handle) {
    std::lock_guard<std::mutex> lock(fb_mutex);
    
    auto it = framebuffers.find(handle);
    if (it == framebuffers.end()) {
        return 0;
    }
    
    return it->second.color_texture;
}

bool FramebufferManager::delete_framebuffer(uint32_t handle) {
    std::lock_guard<std::mutex> lock(fb_mutex);
    
    auto it = framebuffers.find(handle);
    if (it == framebuffers.end()) {
        return false;
    }
    
    Framebuffer& fb = it->second;
    
    // Delete color texture
    if (fb.color_texture != 0) {
        glDeleteTextures(1, &fb.color_texture);
        fb.color_texture = 0;
    }
    
    // Delete depth/stencil texture
    if (fb.depth_stencil_texture != 0) {
        glDeleteTextures(1, &fb.depth_stencil_texture);
        fb.depth_stencil_texture = 0;
    }
    
    // Delete effect texture
    if (fb.effect_texture != 0) {
        glDeleteTextures(1, &fb.effect_texture);
        fb.effect_texture = 0;
    }
    
    // Delete framebuffers
    if (fb.fbo != 0) {
        glDeleteFramebuffers(1, &fb.fbo);
        fb.fbo = 0;
    }
    
    if (fb.effect_fbo != 0) {
        glDeleteFramebuffers(1, &fb.effect_fbo);
        fb.effect_fbo = 0;
    }
    
    framebuffers.erase(it);
    return true;
}

void FramebufferManager::clear_all() {
    std::lock_guard<std::mutex> lock(fb_mutex);
    
    // Delete all framebuffers
    for (auto& pair : framebuffers) {
        Framebuffer& fb = pair.second;
        
        if (fb.color_texture != 0) {
            glDeleteTextures(1, &fb.color_texture);
        }
        
        if (fb.depth_stencil_texture != 0) {
            glDeleteTextures(1, &fb.depth_stencil_texture);
        }
        
        if (fb.effect_texture != 0) {
            glDeleteTextures(1, &fb.effect_texture);
        }
        
        if (fb.fbo != 0) {
            glDeleteFramebuffers(1, &fb.fbo);
        }
        
        if (fb.effect_fbo != 0) {
            glDeleteFramebuffers(1, &fb.effect_fbo);
        }
    }
    
    framebuffers.clear();
    
    // Delete shaders
    for (auto& pair : effect_shaders) {
        if (pair.second != 0) {
            glDeleteProgram(pair.second);
        }
    }
    effect_shaders.clear();
    
    for (auto& pair : scaling_shaders) {
        if (pair.second != 0) {
            glDeleteProgram(pair.second);
        }
    }
    scaling_shaders.clear();
    
    // Delete quad VBO
    if (quad_vbo != 0) {
        glDeleteBuffers(1, &quad_vbo);
        quad_vbo = 0;
    }
}

void FramebufferManager::set_default_size(uint32_t width, uint32_t height) {
    default_width = width;
    default_height = height;
}

void FramebufferManager::set_global_scaling_mode(ScalingMode mode, float factor) {
    global_scaling_mode = mode;
    global_scaling_factor = factor;
}

void FramebufferManager::enable_effects(bool enable) {
    effects_enabled = enable;
}

} // namespace renderer::gl
