#pragma once
#include "IGraphicsBackend.h"
#include "switch_renderer.h"

class RendererGLES3 : public SwitchRenderer {
public:
    RendererGLES3() = default;
    ~RendererGLES3() override = default;
    // Inherit all methods from SwitchRenderer for now
}; 