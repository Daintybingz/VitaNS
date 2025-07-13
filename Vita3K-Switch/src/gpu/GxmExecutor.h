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
                    // Placeholder: draw a blue rectangle for each draw call
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glBegin(GL_TRIANGLES);
                    glColor3f(0.2f, 0.4f, 1.0f);
                    glVertex2f(-0.5f, -0.5f);
                    glVertex2f(0.5f, -0.5f);
                    glVertex2f(0.0f, 0.5f);
                    glEnd();
                    // backend->draw(); // Keep for future real backend
                    break;
                }
                case GxmGpuCommandType::Clear: {
                    auto* clear = static_cast<GxmClearCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing ClearCommand: mask=0x%X\n", clear->clearMask);
                    // Map GXM clear mask to OpenGL ES clear bits
                    GLbitfield mask = 0;
                    if (clear->clearMask & 0x1) mask |= GL_COLOR_BUFFER_BIT;
                    if (clear->clearMask & 0x2) mask |= GL_DEPTH_BUFFER_BIT;
                    if (clear->clearMask & 0x4) mask |= GL_STENCIL_BUFFER_BIT;
                    glClearColor(0.1f, 0.1f, 0.2f, 1.0f); // Vita3K dark blue
                    glClear(mask);
                    break;
                }
                case GxmGpuCommandType::BindTexture: {
                    auto* tex = static_cast<GxmBindTextureCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing BindTexture: unit=%u, id=%u\n", tex->textureUnit, tex->textureId);
                    // TODO: Implement texture binding
                    backend->bindTexture();
                    break;
                }
                case GxmGpuCommandType::BindShader: {
                    auto* shd = static_cast<GxmBindShaderCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing BindShader: name=%s\n", shd->shaderName.c_str());
                    // TODO: Implement shader binding
                    backend->createShader();
                    break;
                }
                case GxmGpuCommandType::SetState: {
                    auto* state = static_cast<GxmSetStateCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing SetState: id=%u, value=%u\n", state->stateId, state->value);
                    // TODO: Update GpuState and backend state
                    break;
                }
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