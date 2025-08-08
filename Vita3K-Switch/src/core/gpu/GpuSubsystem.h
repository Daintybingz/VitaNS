#pragma once
#include "../../renderer/Renderer.h"
#include "../../gpu/GxmExecutor.h"
#include "../../gpu/GxmCommandBuffer.h"
#include <memory>

class GpuSubsystem {
public:
    GpuSubsystem(Renderer* renderer);
    ~GpuSubsystem();
    
    void beginFrame();
    void endFrame();

    // Access to the command buffer for modules to enqueue commands
    GxmCommandBuffer& getCommandBuffer() { return commandBuffer; }
    
private:
    Renderer* renderer;
    std::unique_ptr<GxmExecutor> gxmExecutor;
    GxmCommandBuffer commandBuffer;
}; 