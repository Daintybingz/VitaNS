#pragma once
#include "GxmCommandBuffer.h"
#include "GpuState.h"
#include "../renderer/gl/IGraphicsBackend.h"
#include <cstdio>
#include <SDL2/SDL.h>

class GxmExecutor {
public:
    GxmExecutor(IGraphicsBackend* backend, SDL_Renderer* sdlRenderer = nullptr)
        : backend(backend), sdlRenderer(sdlRenderer) {}
    void execute(const GxmCommandBuffer& buf) {
        for (const auto& cmd : buf.commands) {
            switch (cmd->type) {
                case GxmGpuCommandType::Draw: {
                    auto* draw = static_cast<GxmDrawCall*>(cmd.get());
                    printf("[GxmExecutor] Executing DrawCall: primType=%u, indexType=%u, indexCount=%u\n", draw->primType, draw->indexType, draw->indexCount);
                    // SDL2 placeholder: draw a blue rectangle for each draw call
                    if (sdlRenderer) {
                        SDL_SetRenderDrawColor(sdlRenderer, 50, 100, 220, 255);
                        SDL_Rect rect = {400, 300, 480, 120};
                        SDL_RenderFillRect(sdlRenderer, &rect);
                    }
                    // backend->draw(); // Keep for future real backend
                    break;
                }
                case GxmGpuCommandType::Clear: {
                    auto* clear = static_cast<GxmClearCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing ClearCommand: mask=0x%X\n", clear->clearMask);
                    // For SDL2, just clear the renderer
                    if (sdlRenderer) {
                        SDL_SetRenderDrawColor(sdlRenderer, 20, 20, 40, 255); // Vita3K dark blue
                        SDL_RenderClear(sdlRenderer);
                    }
                    break;
                }
                case GxmGpuCommandType::BindTexture: {
                    auto* tex = static_cast<GxmBindTextureCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing BindTexture: unit=%u, id=%u\n", tex->textureUnit, tex->textureId);
                    backend->bindTexture();
                    break;
                }
                case GxmGpuCommandType::BindShader: {
                    auto* shd = static_cast<GxmBindShaderCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing BindShader: name=%s\n", shd->shaderName.c_str());
                    backend->createShader();
                    break;
                }
                case GxmGpuCommandType::SetState: {
                    auto* state = static_cast<GxmSetStateCommand*>(cmd.get());
                    printf("[GxmExecutor] Executing SetState: id=%u, value=%u\n", state->stateId, state->value);
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
    SDL_Renderer* sdlRenderer;
    GpuState gpuState;
}; 