#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>
#include <array>
#include <algorithm>

namespace shader {

enum class PostEffectType {
    FXAA,               // Fast Approximate Anti-Aliasing
    SMAA,               // Subpixel Morphological Anti-Aliasing
    ColorCorrection,    // Color grading and correction
    Bloom,              // HDR bloom effect
    DOF,                // Depth of field
    MotionBlur,        // Per-object motion blur
    Sharpening,        // Adaptive sharpening
    FilmGrain,         // Film grain overlay
    ChromaticAberration, // Color distortion
    Vignette,          // Edge darkening
    ToneMapping        // HDR to LDR conversion
};

struct PostEffectParams {
    // Anti-aliasing parameters
    float fxaaQualitySubpix = 0.75f;
    float fxaaQualityEdgeThreshold = 0.166f;
    float fxaaQualityEdgeThresholdMin = 0.0833f;
    
    // Color correction
    float contrast = 1.0f;
    float saturation = 1.0f;
    float brightness = 1.0f;
    float gamma = 2.2f;
    
    // Bloom
    float bloomThreshold = 1.0f;
    float bloomIntensity = 1.0f;
    float bloomRadius = 1.0f;
    
    // Depth of field
    float dofFocalDistance = 10.0f;
    float dofFocalRange = 5.0f;
    float dofBlurAmount = 1.0f;
    
    // Motion blur
    float motionBlurStrength = 1.0f;
    float motionBlurSamples = 8.0f;
    
    // Sharpening
    float sharpenStrength = 0.5f;
    float sharpenClamp = 0.1f;
    
    // Film grain
    float filmGrainAmount = 0.025f;
    float filmGrainSpeed = 1.0f;
    
    // Chromatic aberration
    float chromaticAberrationStrength = 0.5f;
    float chromaticAberrationOffset = 2.0f;
    
    // Vignette
    float vignetteIntensity = 0.5f;
    float vignetteSmoothness = 0.5f;
    
    // Tone mapping
    float exposureKey = 0.25f;
    float whitePoint = 11.2f;
};

class PostProcessor {
public:
    PostProcessor();
    ~PostProcessor();

    // Initialization
    bool initialize();
    void shutdown();

    // Effect management
    void enable_effect(PostEffectType effect, bool enabled);
    bool is_effect_enabled(PostEffectType effect) const;
    void set_effect_params(PostEffectType effect, const PostEffectParams& params);
    PostEffectParams get_effect_params(PostEffectType effect) const;

    // Processing
    void process_frame(const void* input_texture, void* output_texture,
                      uint32_t width, uint32_t height,
                      const void* depth_texture = nullptr,
                      const void* motion_vectors = nullptr);

    // Quality settings
    enum class QualityPreset {
        Low,
        Medium,
        High,
        Ultra
    };

    void set_quality_preset(QualityPreset preset);
    void set_custom_quality(float quality_scale);

    // Performance monitoring
    struct PerformanceMetrics {
        float processing_time_ms;
        float memory_usage_mb;
        uint32_t active_effects;
    };

    PerformanceMetrics get_metrics() const;

private:
    struct PostProcessorImpl;
    std::unique_ptr<PostProcessorImpl> impl;

    // Effect chain management
    void setup_effect_chain();
    void update_effect_resources();
    void cleanup_effect_resources();

    // Individual effect processors
    void apply_anti_aliasing(const void* input, void* output);
    void apply_color_correction(const void* input, void* output);
    void apply_bloom(const void* input, void* output);
    void apply_depth_of_field(const void* input, void* output, const void* depth);
    void apply_motion_blur(const void* input, void* output, const void* motion);
    void apply_sharpening(const void* input, void* output);
    void apply_film_grain(const void* input, void* output);
    void apply_chromatic_aberration(const void* input, void* output);
    void apply_vignette(const void* input, void* output);
    void apply_tone_mapping(const void* input, void* output);

    // Resource management
    void allocate_intermediate_buffers(uint32_t width, uint32_t height);
    void release_intermediate_buffers();

    // Helper functions
    void validate_parameters();
    void optimize_effect_order();
    void update_quality_settings();
};

} // namespace shader 