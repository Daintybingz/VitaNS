#include "post_processor.h"
#include <unordered_map>
#include <array>
#include <algorithm>
#include <cstring>
#include <chrono>

namespace shader {

struct PostProcessor::PostProcessorImpl {
    std::unordered_map<PostEffectType, bool> enabled_effects;
    std::unordered_map<PostEffectType, PostEffectParams> effect_params;
    std::vector<PostEffectType> effect_chain;
    QualityPreset quality_preset;
    float quality_scale;
    PerformanceMetrics metrics;
    
    // Intermediate buffers
    struct Buffer {
        void* data;
        uint32_t width;
        uint32_t height;
        uint32_t size;
    };
    std::vector<Buffer> intermediate_buffers;
    
    PostProcessorImpl() : quality_preset(QualityPreset::High), quality_scale(1.0f) {
        // Initialize default enabled effects
        enabled_effects[PostEffectType::FXAA] = true;
        enabled_effects[PostEffectType::ColorCorrection] = true;
        enabled_effects[PostEffectType::ToneMapping] = true;
        
        // Initialize default parameters
        effect_params[PostEffectType::FXAA] = PostEffectParams{};
        effect_params[PostEffectType::ColorCorrection] = PostEffectParams{};
        effect_params[PostEffectType::ToneMapping] = PostEffectParams{};
        
        metrics = {
            .processing_time_ms = 0.0f,
            .memory_usage_mb = 0.0f,
            .active_effects = 0
        };
    }
};

PostProcessor::PostProcessor() : impl(std::make_unique<PostProcessorImpl>()) {}

PostProcessor::~PostProcessor() {
    shutdown();
}

bool PostProcessor::initialize() {
    setup_effect_chain();
    return true;
}

void PostProcessor::shutdown() {
    cleanup_effect_resources();
    release_intermediate_buffers();
    impl->enabled_effects.clear();
    impl->effect_params.clear();
    impl->effect_chain.clear();
}

void PostProcessor::enable_effect(PostEffectType effect, bool enabled) {
    impl->enabled_effects[effect] = enabled;
    setup_effect_chain();
}

bool PostProcessor::is_effect_enabled(PostEffectType effect) const {
    auto it = impl->enabled_effects.find(effect);
    return it != impl->enabled_effects.end() && it->second;
}

void PostProcessor::set_effect_params(PostEffectType effect, const PostEffectParams& params) {
    impl->effect_params[effect] = params;
    validate_parameters();
}

PostEffectParams PostProcessor::get_effect_params(PostEffectType effect) const {
    auto it = impl->effect_params.find(effect);
    return it != impl->effect_params.end() ? it->second : PostEffectParams{};
}

void PostProcessor::process_frame(const void* input_texture, void* output_texture,
                                uint32_t width, uint32_t height,
                                const void* depth_texture,
                                const void* motion_vectors) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Allocate or resize intermediate buffers if needed
    allocate_intermediate_buffers(width, height);
    
    // Process each effect in the chain
    const void* current_input = input_texture;
    void* current_output = nullptr;
    
    for (size_t i = 0; i < impl->effect_chain.size(); i++) {
        current_output = (i == impl->effect_chain.size() - 1) ? 
                        output_texture : impl->intermediate_buffers[i].data;
        
        auto effect = impl->effect_chain[i];
        switch (effect) {
            case PostEffectType::FXAA:
                apply_anti_aliasing(current_input, current_output);
                break;
            case PostEffectType::SMAA:
                apply_anti_aliasing(current_input, current_output);
                break;
            case PostEffectType::ColorCorrection:
                apply_color_correction(current_input, current_output);
                break;
            case PostEffectType::Bloom:
                apply_bloom(current_input, current_output);
                break;
            case PostEffectType::DOF:
                apply_depth_of_field(current_input, current_output, depth_texture);
                break;
            case PostEffectType::MotionBlur:
                apply_motion_blur(current_input, current_output, motion_vectors);
                break;
            case PostEffectType::Sharpening:
                apply_sharpening(current_input, current_output);
                break;
            case PostEffectType::FilmGrain:
                apply_film_grain(current_input, current_output);
                break;
            case PostEffectType::ChromaticAberration:
                apply_chromatic_aberration(current_input, current_output);
                break;
            case PostEffectType::Vignette:
                apply_vignette(current_input, current_output);
                break;
            case PostEffectType::ToneMapping:
                apply_tone_mapping(current_input, current_output);
                break;
        }
        
        current_input = current_output;
    }
    
    // Update metrics
    auto end_time = std::chrono::high_resolution_clock::now();
    impl->metrics.processing_time_ms = std::chrono::duration<float, std::milli>(
        end_time - start_time).count();
    impl->metrics.active_effects = impl->effect_chain.size();
}

void PostProcessor::set_quality_preset(QualityPreset preset) {
    impl->quality_preset = preset;
    update_quality_settings();
}

void PostProcessor::set_custom_quality(float quality_scale) {
    impl->quality_scale = std::clamp(quality_scale, 0.1f, 2.0f);
    update_quality_settings();
}

PostProcessor::PerformanceMetrics PostProcessor::get_metrics() const {
    return impl->metrics;
}

// Private methods

void PostProcessor::setup_effect_chain() {
    impl->effect_chain.clear();
    
    // Define optimal effect order
    const std::array<PostEffectType, 11> optimal_order = {
        PostEffectType::MotionBlur,
        PostEffectType::DOF,
        PostEffectType::Bloom,
        PostEffectType::ColorCorrection,
        PostEffectType::ToneMapping,
        PostEffectType::ChromaticAberration,
        PostEffectType::FXAA,
        PostEffectType::SMAA,
        PostEffectType::Sharpening,
        PostEffectType::FilmGrain,
        PostEffectType::Vignette
    };
    
    // Add enabled effects in optimal order
    for (auto effect : optimal_order) {
        if (is_effect_enabled(effect)) {
            impl->effect_chain.push_back(effect);
        }
    }
}

void PostProcessor::update_effect_resources() {
    // Update resources based on quality settings
    for (auto& [effect, params] : impl->effect_params) {
        switch (effect) {
            case PostEffectType::FXAA:
                params.fxaaQualitySubpix *= impl->quality_scale;
                break;
            case PostEffectType::Bloom:
                params.bloomRadius *= impl->quality_scale;
                break;
            case PostEffectType::DOF:
                params.dofBlurAmount *= impl->quality_scale;
                break;
            case PostEffectType::MotionBlur:
                params.motionBlurSamples = std::max(4.0f, 
                    params.motionBlurSamples * impl->quality_scale);
                break;
            default:
                break;
        }
    }
}

void PostProcessor::cleanup_effect_resources() {
    // Clean up any effect-specific resources
    release_intermediate_buffers();
}

void PostProcessor::apply_anti_aliasing(const void* input, void* output) {
    // Stub out FXAA - not implemented for Switch
    // Simply copy input to output without anti-aliasing
    if (input && output) {
        // Note: This is a stub - we don't have width/height here
        // In a real implementation, these would be passed as parameters
        // For now, we'll just return without doing anything
        return;
    }
}

void PostProcessor::apply_color_correction(const void* input, void* output) {
    // TODO: Implement color correction
}

void PostProcessor::apply_bloom(const void* input, void* output) {
    // TODO: Implement bloom
}

void PostProcessor::apply_depth_of_field(const void* input, void* output, const void* depth) {
    // TODO: Implement depth of field
}

void PostProcessor::apply_motion_blur(const void* input, void* output, const void* motion) {
    // TODO: Implement motion blur
}

void PostProcessor::apply_sharpening(const void* input, void* output) {
    // TODO: Implement sharpening
}

void PostProcessor::apply_film_grain(const void* input, void* output) {
    // TODO: Implement film grain
}

void PostProcessor::apply_chromatic_aberration(const void* input, void* output) {
    // TODO: Implement chromatic aberration
}

void PostProcessor::apply_vignette(const void* input, void* output) {
    // TODO: Implement vignette
}

void PostProcessor::apply_tone_mapping(const void* input, void* output) {
    // TODO: Implement tone mapping
}

void PostProcessor::allocate_intermediate_buffers(uint32_t width, uint32_t height) {
    size_t required_buffers = impl->effect_chain.size() > 1 ? 
                             impl->effect_chain.size() - 1 : 0;
    
    // Resize buffer pool if needed
    if (impl->intermediate_buffers.size() < required_buffers) {
        impl->intermediate_buffers.resize(required_buffers);
    }
    
    // Allocate or resize buffers
    size_t buffer_size = width * height * 4; // Assuming RGBA8
    for (auto& buffer : impl->intermediate_buffers) {
        if (!buffer.data || buffer.width != width || buffer.height != height) {
            free(buffer.data);
            buffer.data = malloc(buffer_size);
            buffer.width = width;
            buffer.height = height;
            buffer.size = buffer_size;
        }
    }
    
    // Update memory usage metric
    impl->metrics.memory_usage_mb = (buffer_size * impl->intermediate_buffers.size()) 
                                  / (1024.0f * 1024.0f);
}

void PostProcessor::release_intermediate_buffers() {
    for (auto& buffer : impl->intermediate_buffers) {
        free(buffer.data);
        buffer.data = nullptr;
        buffer.size = 0;
    }
    impl->intermediate_buffers.clear();
    impl->metrics.memory_usage_mb = 0.0f;
}

void PostProcessor::validate_parameters() {
    for (auto& [effect, params] : impl->effect_params) {
        switch (effect) {
            case PostEffectType::FXAA:
                params.fxaaQualitySubpix = std::clamp(params.fxaaQualitySubpix, 0.0f, 1.0f);
                params.fxaaQualityEdgeThreshold = std::clamp(params.fxaaQualityEdgeThreshold, 0.063f, 0.333f);
                params.fxaaQualityEdgeThresholdMin = std::clamp(params.fxaaQualityEdgeThresholdMin, 0.0312f, 0.0833f);
                break;
            case PostEffectType::ColorCorrection:
                params.contrast = std::clamp(params.contrast, 0.0f, 2.0f);
                params.saturation = std::clamp(params.saturation, 0.0f, 2.0f);
                params.brightness = std::clamp(params.brightness, 0.0f, 2.0f);
                params.gamma = std::clamp(params.gamma, 1.0f, 3.0f);
                break;
            // Add validation for other effects...
        }
    }
}

void PostProcessor::optimize_effect_order() {
    // Optimize effect order based on dependencies and performance
    std::sort(impl->effect_chain.begin(), impl->effect_chain.end(),
        [](PostEffectType a, PostEffectType b) {
            // Define effect priorities (lower number = higher priority)
            std::unordered_map<PostEffectType, int> priorities = {
                {PostEffectType::MotionBlur, 0},
                {PostEffectType::DOF, 1},
                {PostEffectType::Bloom, 2},
                {PostEffectType::ColorCorrection, 3},
                {PostEffectType::ToneMapping, 4},
                {PostEffectType::ChromaticAberration, 5},
                {PostEffectType::FXAA, 6},
                {PostEffectType::SMAA, 7},
                {PostEffectType::Sharpening, 8},
                {PostEffectType::FilmGrain, 9},
                {PostEffectType::Vignette, 10}
            };
            return priorities[a] < priorities[b];
        });
}

void PostProcessor::update_quality_settings() {
    // Set quality scale based on preset
    switch (impl->quality_preset) {
        case QualityPreset::Low:
            impl->quality_scale = 0.5f;
            break;
        case QualityPreset::Medium:
            impl->quality_scale = 0.75f;
            break;
        case QualityPreset::High:
            impl->quality_scale = 1.0f;
            break;
        case QualityPreset::Ultra:
            impl->quality_scale = 1.5f;
            break;
    }
    
    update_effect_resources();
}

} // namespace shader 