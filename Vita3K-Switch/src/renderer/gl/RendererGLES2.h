#pragma once
#include "IGraphicsBackend.h"
#include "switch_renderer.h"

class RendererGLES2 : public SwitchRenderer {
public:
    RendererGLES2() = default;
    ~RendererGLES2() override = default;
    // Inherit all methods from SwitchRenderer for now
}; 