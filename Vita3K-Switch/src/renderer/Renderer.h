#pragma once

// Abstract interface for all renderer backends
class Renderer {
public:
    virtual ~Renderer() = default;
    virtual bool init() = 0;
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void draw_frame(/* const FrameData& frame */) = 0;
    virtual void present() = 0;
    virtual void shutdown() = 0;
}; 