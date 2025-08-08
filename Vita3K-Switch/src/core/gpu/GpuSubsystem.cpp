#include "GpuSubsystem.h"
#include "../../renderer/Renderer.h"
#include "../../gpu/GxmExecutor.h"
#include "../../gpu/GxmCommandBuffer.h"

GpuSubsystem::GpuSubsystem(Renderer* renderer) : renderer(renderer) {
    // Initialize GXM command parsing
    gxmExecutor = std::make_unique<GxmExecutor>(renderer);
    printf("[GpuSubsystem] Initialized with renderer\n");
}

// Execution of queued commands would be triggered by begin/end frame or elsewhere as needed

void GpuSubsystem::beginFrame() {
    if (renderer) {
        renderer->beginFrame();
    }
}

void GpuSubsystem::endFrame() {
    if (renderer) {
        renderer->endFrame();
    }
}

GpuSubsystem::~GpuSubsystem() = default; 