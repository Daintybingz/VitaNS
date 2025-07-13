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
                case GxmGpuCommandType::Draw: {
                    auto* draw = static_cast<GxmDrawCall*>(cmd.get());
                    printf("[GxmExecutor] Executing DrawCall: primType=%u, indexType=%u, indexCount=%u\n", draw->primType, draw->indexType, draw->indexCount);
                    backend->draw();
                    break;
                }
                case GxmGpuCommandType::Clear: {
                    auto* clear = static_cast<GxmClearCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing ClearCommand: mask=0x%X\n", clear->clearMask);
                    // TODO: Call backend clear method
                    break;
                }
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
                    printf("[GxmExecutor] Unknown command type!\n");
                    break;
            }
        }
    }
private:
    IGraphicsBackend* backend;
    GpuState gpuState;
}; 