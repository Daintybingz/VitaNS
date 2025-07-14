#pragma once
#include "GxmCommandBuffer.h"
#include "GpuState.h"
#include "../renderer/RendererGLES2.h"
#include <cstdio>
#include <SDL2/SDL.h>

class GxmExecutor {
public:
    GxmExecutor(RendererGLES2* renderer, SDL_Renderer* sdlRenderer = nullptr)
        : renderer(renderer), sdlRenderer(sdlRenderer) {}
    void execute(const GxmCommandBuffer& buf) {
        for (const auto& cmd : buf.commands) {
            switch (cmd->type) {
                case GxmGpuCommandType::Draw: {
                    auto* draw = static_cast<GxmDrawCall*>(cmd.get());
                    printf("[GxmExecutor] Executing DrawCall: primType=%u, indexType=%u, indexCount=%u\n", draw->primType, draw->indexType, draw->indexCount);
                    renderer->draw(draw->primType, draw->indexType, draw->indexData, draw->indexCount);
                    break;
                }
                case GxmGpuCommandType::Clear: {
                    auto* clear = static_cast<GxmClearCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing ClearCommand: mask=0x%X\n", clear->clearMask);
                    renderer->clear(clear->clearMask);
                    break;
                }
                case GxmGpuCommandType::BindTexture: {
                    auto* tex = static_cast<GxmBindTextureCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing BindTexture: unit=%u, id=%u\n", tex->textureUnit, tex->textureId);
                    renderer->bindTexture(tex->textureUnit, tex->textureId);
                    break;
                }
                case GxmGpuCommandType::BindShader: {
                    auto* shd = static_cast<GxmBindShaderCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing BindShader: name=%s\n", shd->shaderName.c_str());
                    renderer->createShader(shd->shaderName);
                    break;
                }
                case GxmGpuCommandType::SetState: {
                    auto* state = static_cast<GxmSetStateCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing SetState: id=%u, value=%u\n", state->stateId, state->value);
                    renderer->setState(state->stateId, state->value);
                    break;
                }
                case GxmGpuCommandType::UploadVertexBuffer: {
                    auto* vb = static_cast<GxmUploadVertexBufferCommand*>(cmd.get());
                    printf("[GxmExecutor] UploadVertexBuffer: size=%zu\n", vb->size);
                    renderer->uploadVertexBuffer(vb->data, vb->size);
                    break;
                }
                case GxmGpuCommandType::UploadIndexBuffer: {
                    auto* ib = static_cast<GxmUploadIndexBufferCommand*>(cmd.get());
                    printf("[GxmExecutor] UploadIndexBuffer: size=%zu, indexType=%u\n", ib->size, ib->indexType);
                    renderer->uploadIndexBuffer(ib->data, ib->size, ib->indexType);
                    break;
                }
                case GxmGpuCommandType::UploadShader: {
                    auto* sh = static_cast<GxmUploadShaderCommand*>(cmd.get());
                    printf("[GxmExecutor] UploadShader: name=%s\n", sh->name.c_str());
                    renderer->createShaderFromSource(sh->name, sh->vertSrc, sh->fragSrc);
                    break;
                }
                case GxmGpuCommandType::SetupVertexAttributes: {
                    auto* attr = static_cast<GxmSetupVertexAttributesCommand*>(cmd.get());
                    printf("[GxmExecutor] SetupVertexAttributes: stride=%zu, count=%zu\n", attr->stride, attr->layout.size());
                    renderer->setupVertexAttributesDynamic(attr->layout, attr->stride);
                    break;
                }
                case GxmGpuCommandType::SetUniform: {
                    auto* uni = static_cast<GxmSetUniformCommand*>(cmd.get());
                    printf("[GxmExecutor] SetUniform: %s\n", uni->name.c_str());
                    if (uni->values.size() == 1) renderer->setUniform(uni->name, uni->values[0]);
                    else renderer->setUniformVec(uni->name, uni->values.data(), uni->values.size());
                    break;
                }
                case GxmGpuCommandType::SetSampler: {
                    auto* smp = static_cast<GxmSetSamplerCommand*>(cmd.get());
                    printf("[GxmExecutor] SetSampler: %s = %d\n", smp->name.c_str(), smp->unit);
                    renderer->setSampler(smp->name, smp->unit);
                    break;
                }
                default:
                    printf("[GxmExecutor] Unknown command type!\n");
                    break;
            }
        }
    }
private:
    RendererGLES2* renderer;
    SDL_Renderer* sdlRenderer;
    GpuState gpuState;
}; 