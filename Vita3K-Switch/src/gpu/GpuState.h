#pragma once
#include <cstdint>
#include <string>

struct GpuState {
    // Blend state
    bool blendEnabled = false;
    uint32_t blendSrc = 0;
    uint32_t blendDst = 0;
    // Depth/stencil
    bool depthTestEnabled = false;
    uint32_t depthFunc = 0;
    bool stencilTestEnabled = false;
    uint32_t stencilFunc = 0;
    // Scissor/viewport
    int viewportX = 0, viewportY = 0, viewportW = 0, viewportH = 0;
    int scissorX = 0, scissorY = 0, scissorW = 0, scissorH = 0;
    // Shader bindings
    std::string vertexShader;
    std::string fragmentShader;
    // Add more as needed

    bool operator==(const GpuState& other) const {
        return blendEnabled == other.blendEnabled &&
               blendSrc == other.blendSrc &&
               blendDst == other.blendDst &&
               depthTestEnabled == other.depthTestEnabled &&
               depthFunc == other.depthFunc &&
               stencilTestEnabled == other.stencilTestEnabled &&
               stencilFunc == other.stencilFunc &&
               viewportX == other.viewportX && viewportY == other.viewportY &&
               viewportW == other.viewportW && viewportH == other.viewportH &&
               scissorX == other.scissorX && scissorY == other.scissorY &&
               scissorW == other.scissorW && scissorH == other.scissorH &&
               vertexShader == other.vertexShader &&
               fragmentShader == other.fragmentShader;
    }
    bool operator!=(const GpuState& other) const { return !(*this == other); }
}; 