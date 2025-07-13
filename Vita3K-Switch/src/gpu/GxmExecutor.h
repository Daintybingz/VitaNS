#pragma once
#include "GxmCommandBuffer.h"
#include "GpuState.h"
#include "../renderer/gl/IGraphicsBackend.h"
#include <cstdio>

class GxmExecutor {
public:
    GxmExecutor(IGraphicsBackend* backend) : backend(backend) {}
    void execute(const GxmCommandBuffer& buf) {
        for (const auto& cmd : buf.commands) {
            switch (cmd->type) {
                case GxmGpuCommandType::Draw:
                    printf("[GxmExecutor] Executing DrawCall\n");
                    backend->draw();
                    break;
                case GxmGpuCommandType::Clear:
                    printf("[GxmExecutor] Executing ClearCommand\n");
                    // TODO: Call backend clear method
                    break;
                case GxmGpuCommandType::BindTexture:
                    printf("[GxmExecutor] Executing BindTextureCommand\n");
                    backend->bindTexture();
                    break;
                case GxmGpuCommandType::BindShader:
                    printf("[GxmExecutor] Executing BindShaderCommand\n");
                    backend->createShader();
                    break;
                case GxmGpuCommandType::SetState:
                    printf("[GxmExecutor] Executing SetStateCommand\n");
                    // TODO: Update GpuState and backend state
                    break;
                default:
                    printf("[GxmExecutor] Unknown command type\n");
                    break;
            }
        }
    }
private:
    IGraphicsBackend* backend;
    GpuState gpuState;
}; 