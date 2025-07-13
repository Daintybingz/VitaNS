#pragma once
#include <string>
#include "GraphicsCapabilities.h"

class IGraphicsBackend {
public:
    virtual ~IGraphicsBackend() = default;
    virtual bool initialize(const std::string& windowTitle, int width, int height) = 0;
    virtual void finalize() = 0;
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void swapBuffers() = 0;
    // Shader/texture/framebuffer management (stubs for now)
    virtual void createShader() = 0;
    virtual void bindTexture() = 0;
    virtual void setFramebuffer() = 0;
    virtual void draw() = 0;
    virtual const GraphicsCapabilities& getCapabilities() const = 0;
}; 