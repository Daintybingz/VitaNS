#include "fxaa.h"
#include <memory>
#include <algorithm>
#include <cstring>
#include <cmath>

namespace shader {
namespace post_effects {

struct FXAA::FXAAImpl {
    // Intermediate buffers
    std::unique_ptr<float[]> edge_map;
    std::unique_ptr<uint8_t[]> temp_buffer;
    uint32_t buffer_width;
    uint32_t buffer_height;
    
    FXAAImpl() : buffer_width(0), buffer_height(0) {}
    
    void ensure_buffer_size(uint32_t width, uint32_t height) {
        if (width != buffer_width || height != buffer_height) {
            edge_map = std::make_unique<float[]>(width * height);
            temp_buffer = std::make_unique<uint8_t[]>(width * height * 4);
            buffer_width = width;
            buffer_height = height;
        }
    }
};

FXAA::FXAA() : impl(std::make_unique<FXAAImpl>()) {}

FXAA::~FXAA() {
    shutdown();
}

bool FXAA::initialize() {
    return true;
}

void FXAA::shutdown() {
    impl->edge_map.reset();
    impl->temp_buffer.reset();
    impl->buffer_width = 0;
    impl->buffer_height = 0;
}

void FXAA::apply(const void* input, void* output,
                 uint32_t width, uint32_t height,
                 const PostEffectParams& params) {
    // Ensure buffers are allocated
    impl->ensure_buffer_size(width, height);
    
    const uint8_t* input_pixels = static_cast<const uint8_t*>(input);
    uint8_t* output_pixels = static_cast<uint8_t*>(output);
    
    // Step 1: Edge detection
    detect_edges(input_pixels, impl->edge_map.get(), width, height);
    
    // Step 2: Apply FXAA
    blend_pixels(input_pixels, output_pixels, impl->edge_map.get(),
                width, height, params);
}

float FXAA::compute_luma(const uint8_t* rgb) {
    // Convert RGB to luminance using Rec. 709 coefficients
    return (0.2126f * rgb[0] + 0.7152f * rgb[1] + 0.0722f * rgb[2]) / 255.0f;
}

void FXAA::detect_edges(const uint8_t* input, float* edge_map,
                       uint32_t width, uint32_t height) {
    // Compute luminance for each pixel
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            const uint8_t* pixel = &input[(y * width + x) * 4];
            edge_map[y * width + x] = compute_luma(pixel);
        }
    }
    
    // Detect edges using local contrast
    for (uint32_t y = 1; y < height - 1; y++) {
        for (uint32_t x = 1; x < width - 1; x++) {
            float center = edge_map[y * width + x];
            
            // Sample neighboring pixels
            float top = edge_map[(y - 1) * width + x];
            float bottom = edge_map[(y + 1) * width + x];
            float left = edge_map[y * width + (x - 1)];
            float right = edge_map[y * width + (x + 1)];
            
            // Compute local contrast
            float min_luma = std::min({top, bottom, left, right, center});
            float max_luma = std::max({top, bottom, left, right, center});
            float contrast = max_luma - min_luma;
            
            // Store edge strength
            edge_map[y * width + x] = std::clamp(
                contrast / std::max(max_luma, 0.0001f),
                0.0f, 1.0f);
        }
    }
}

void FXAA::blend_pixels(const uint8_t* input, uint8_t* output,
                       const float* edge_map,
                       uint32_t width, uint32_t height,
                       const PostEffectParams& params) {
    // FXAA parameters
    float edge_threshold = std::clamp(
        params.fxaaQualityEdgeThreshold,
        EDGE_THRESHOLD_MIN,
        EDGE_THRESHOLD_MAX);
    
    float subpix_quality = std::clamp(
        params.fxaaQualitySubpix,
        SUBPIX_QUALITY_MIN,
        SUBPIX_QUALITY_MAX);
    
    // Process each pixel
    for (uint32_t y = 1; y < height - 1; y++) {
        for (uint32_t x = 1; x < width - 1; x++) {
            size_t pixel_idx = (y * width + x) * 4;
            float edge_strength = edge_map[y * width + x];
            
            if (edge_strength > edge_threshold) {
                // Sample neighboring pixels
                const uint8_t* center = &input[pixel_idx];
                const uint8_t* top = &input[(y - 1) * width * 4 + x * 4];
                const uint8_t* bottom = &input[(y + 1) * width * 4 + x * 4];
                const uint8_t* left = &input[y * width * 4 + (x - 1) * 4];
                const uint8_t* right = &input[y * width * 4 + (x + 1) * 4];
                
                // Compute gradients
                float grad_vert = std::abs(compute_luma(top) - compute_luma(bottom));
                float grad_horz = std::abs(compute_luma(left) - compute_luma(right));
                
                // Determine blur direction
                bool is_horizontal = grad_vert > grad_horz;
                
                // Compute blur weights
                float blur_strength = edge_strength * subpix_quality;
                
                // Apply directional blur
                for (int i = 0; i < 3; i++) {
                    float sum = center[i];
                    float weight = 1.0f;
                    
                    if (is_horizontal) {
                        sum += left[i] * blur_strength;
                        sum += right[i] * blur_strength;
                        weight += 2.0f * blur_strength;
                    } else {
                        sum += top[i] * blur_strength;
                        sum += bottom[i] * blur_strength;
                        weight += 2.0f * blur_strength;
                    }
                    
                    output[pixel_idx + i] = static_cast<uint8_t>(sum / weight);
                }
                output[pixel_idx + 3] = center[3]; // Preserve alpha
            } else {
                // No edge detected, copy original pixel
                std::memcpy(&output[pixel_idx], &input[pixel_idx], 4);
            }
        }
    }
    
    // Copy border pixels unchanged
    for (uint32_t x = 0; x < width; x++) {
        std::memcpy(&output[x * 4], &input[x * 4], 4);
        std::memcpy(&output[(height - 1) * width * 4 + x * 4],
                   &input[(height - 1) * width * 4 + x * 4], 4);
    }
    for (uint32_t y = 1; y < height - 1; y++) {
        std::memcpy(&output[y * width * 4], &input[y * width * 4], 4);
        std::memcpy(&output[y * width * 4 + (width - 1) * 4],
                   &input[y * width * 4 + (width - 1) * 4], 4);
    }
}

} // namespace post_effects
} // namespace shader 