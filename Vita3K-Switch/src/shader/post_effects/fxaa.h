#pragma once

#include <cstdint>
#include "../post_processor.h"

namespace shader {
namespace post_effects {

class FXAA {
public:
    FXAA();
    ~FXAA();

    bool initialize();
    void shutdown();

    void apply(const void* input, void* output,
               uint32_t width, uint32_t height,
               const PostEffectParams& params);

private:
    struct FXAAImpl;
    std::unique_ptr<FXAAImpl> impl;

    // FXAA algorithm parameters
    static constexpr float EDGE_THRESHOLD_MIN = 0.0312f;
    static constexpr float EDGE_THRESHOLD_MAX = 0.125f;
    static constexpr float SUBPIX_QUALITY_MIN = 0.0f;
    static constexpr float SUBPIX_QUALITY_MAX = 1.0f;
    static constexpr int ITERATIONS = 12;

    // Helper functions
    float compute_luma(const uint8_t* rgb);
    void detect_edges(const uint8_t* input, float* edge_map,
                     uint32_t width, uint32_t height);
    void blend_pixels(const uint8_t* input, uint8_t* output,
                     const float* edge_map,
                     uint32_t width, uint32_t height,
                     const PostEffectParams& params);
};

} // namespace post_effects
} // namespace shader 