#pragma once
#include "../../renderer/Renderer.h"
#include "../../modules/SceGxm/GxmExecutor.h"
#include <memory>

class GpuSubsystem {
public:
    GpuSubsystem(Renderer* renderer);
    ~GpuSubsystem();
    
    void beginFrame();
    void endFrame();
    void executeGxmCommand(const GxmCommand& cmd);
    
private:
    Renderer* renderer;
    std::unique_ptr<GxmExecutor> gxmExecutor;
}; 