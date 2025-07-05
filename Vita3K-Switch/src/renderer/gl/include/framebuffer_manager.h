#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <GLES2/gl2.h>
#include <SDL2/SDL_opengles2.h>

namespace renderer::gl {

enum class ScalingMode {
    None,        // No scaling
    Bilinear,    // Bilinear filtering
    Bicubic,     // Bicubic filtering
    FSR,         // AMD FidelityFX Super Resolution
    Custom       // Custom shader-based scaling
};

enum class FramebufferEffect {
    None,           // No effect
    Blur,           // Gaussian blur
    ColorCorrection,// Color correction
    Bloom,          // Bloom effect
    Custom          // Custom shader-based effect
};

struct FramebufferParams {
    uint32_t width;
    uint32_t height;
    uint32_t format;
    bool has_depth;
    bool has_stencil;
    ScalingMode scaling;
    float scaling_factor;
    FramebufferEffect effect;
};

struct Framebuffer {
    GLuint fbo;
    GLuint color_texture;
    GLuint depth_stencil_texture;
    uint32_t width;
    uint32_t height;
    uint32_t scaled_width;
    uint32_t scaled_height;
    ScalingMode scaling;
    FramebufferEffect effect;
    bool is_multisampled;
    GLuint effect_fbo;      // For post-processing effects
    GLuint effect_texture;
};

class FramebufferManager {
public:
    FramebufferManager();
    ~FramebufferManager();

    // Initialize the framebuffer manager
    bool initialize();

    // Create a framebuffer with the given parameters
    uint32_t create_framebuffer(const FramebufferParams& params);

    // Bind a framebuffer for rendering
    bool bind_framebuffer(uint32_t handle);

    // Resolve a multisampled framebuffer
    bool resolve_framebuffer(uint32_t handle);

    // Apply post-processing effects to a framebuffer
    bool apply_effects(uint32_t handle);

    // Scale a framebuffer to the target size
    bool scale_framebuffer(uint32_t handle, uint32_t target_width, uint32_t target_height);

    // Get the texture handle for a framebuffer
    GLuint get_color_texture(uint32_t handle);

    // Delete a framebuffer
    bool delete_framebuffer(uint32_t handle);

    // Clear all framebuffers
    void clear_all();

    // Set the default framebuffer size (for docked/handheld mode)
    void set_default_size(uint32_t width, uint32_t height);

    // Set global scaling mode
    void set_global_scaling_mode(ScalingMode mode, float factor);

    // Enable/disable framebuffer effects
    void enable_effects(bool enable);
    
    // Memory optimization functions
    void optimize_memory_usage();
    void log_memory_usage();
    void update_usage_tracking(uint32_t handle);

private:
    // Apply a specific effect to a framebuffer
    bool apply_effect(Framebuffer& fb, FramebufferEffect effect);

    // Create a shader for a specific effect
    GLuint create_effect_shader(FramebufferEffect effect);

    // Create a scaling shader for a specific mode
    GLuint create_scaling_shader(ScalingMode mode);

    // Internal framebuffer creation
    bool create_framebuffer_internal(Framebuffer& fb, uint32_t width, uint32_t height, 
                                    uint32_t format, bool has_depth, bool has_stencil);

    // Create MSAA framebuffer
    bool create_msaa_framebuffer(Framebuffer& fb);

    // Create effect framebuffer
    bool create_effect_framebuffer(Framebuffer& fb);
    
    // Memory optimization helpers
    void optimize_by_usage_pattern();
    void reduce_msaa_level();
    void disable_msaa(uint32_t handle);
    void share_depth_buffers();
    void reduce_offscreen_resolution();

    std::unordered_map<uint32_t, Framebuffer> framebuffers;
    std::mutex fb_mutex;

    uint32_t next_handle;
    uint32_t default_width;
    uint32_t default_height;
    ScalingMode global_scaling_mode;
    float global_scaling_factor;
    bool effects_enabled;

    // Shader programs for effects and scaling
    std::unordered_map<FramebufferEffect, GLuint> effect_shaders;
    std::unordered_map<ScalingMode, GLuint> scaling_shaders;

    // Full-screen quad for post-processing
    GLuint quad_vbo;
};

} // namespace renderer::gl
