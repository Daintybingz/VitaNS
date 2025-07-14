#include "GpuSubsystem.h"
#include "../../renderer/Renderer.h"
#include "../../modules/SceGxm/GxmExecutor.h"

GpuSubsystem::GpuSubsystem(Renderer* renderer) : renderer(renderer) {
    // Initialize GXM command parsing
    gxmExecutor = std::make_unique<GxmExecutor>(renderer);
    printf("[GpuSubsystem] Initialized with renderer\n");
}

void GpuSubsystem::executeGxmCommand(const GxmCommand& cmd) {
    if (gxmExecutor) {
        gxmExecutor->executeCommand(cmd);
    }
} 