#pragma once
#include "GxmCommandQueue.h"
#include "GxmExecutor.h"
#include "../renderer/gl/IGraphicsBackend.h"
#include <memory>
#include "../core/emulator/emulator.h"
#include "../../modules/SceGxm/SceGxm.h"

class GpuSubsystem {
public:
    GpuSubsystem(IGraphicsBackend* backend, SDL_Renderer* sdlRenderer = nullptr)
        : executor(backend, sdlRenderer) {}

    // Stub: get the current GXM command buffer from emulated memory
    // TODO: Replace with real memory manager/GXM module integration
    bool getCurrentGxmCommandBuffer(const uint8_t*& data, size_t& size) {
        SceGxm* gxm = Emulator::getInstance().getSceGxm();
        if (!gxm || !gxm->currentContext) return false;
        data = reinterpret_cast<const uint8_t*>(gxm->currentContext->commandBuffer);
        size = gxm->currentContext->commandBufferSize;
        return (data && size > 0);
    }

    // Parse a GXM command stream (minimal real parser)
    void parseGxmCommandStream(const uint8_t* data, size_t size, GxmCommandBuffer& outBuf) {
        if (!data || size < 4) return;
        size_t offset = 0;
        while (offset + 4 <= size) {
            uint32_t cmd = *reinterpret_cast<const uint32_t*>(data + offset);
            offset += 4;
            if (cmd == 0x01) { // Clear
                if (offset + 4 > size) break;
                uint32_t mask = *reinterpret_cast<const uint32_t*>(data + offset);
                offset += 4;
                auto clearCmd = std::make_unique<GxmClearCommand>();
                clearCmd->clearMask = mask;
                printf("[GXM Parse] ClearCommand: mask=0x%X\n", mask);
                outBuf.add(std::move(clearCmd));
            } else if (cmd == 0x02) { // Draw
                if (offset + 12 > size) break;
                uint32_t primType = *reinterpret_cast<const uint32_t*>(data + offset);
                uint32_t indexType = *reinterpret_cast<const uint32_t*>(data + offset + 4);
                uint32_t indexCount = *reinterpret_cast<const uint32_t*>(data + offset + 8);
                offset += 12;
                auto drawCmd = std::make_unique<GxmDrawCall>();
                drawCmd->primType = primType;
                drawCmd->indexType = indexType;
                drawCmd->indexData = nullptr;
                drawCmd->indexCount = indexCount;
                printf("[GXM Parse] DrawCall: primType=%u, indexType=%u, indexCount=%u\n", primType, indexType, indexCount);
                outBuf.add(std::move(drawCmd));
            } else if (cmd == 0x03) { // BindTexture
                if (offset + 8 > size) break;
                uint32_t unit = *reinterpret_cast<const uint32_t*>(data + offset);
                uint32_t id = *reinterpret_cast<const uint32_t*>(data + offset + 4);
                offset += 8;
                auto texCmd = std::make_unique<GxmBindTextureCommand>();
                texCmd->textureUnit = unit;
                texCmd->textureId = id;
                printf("[GXM Parse] BindTexture: unit=%u, id=%u\n", unit, id);
                outBuf.add(std::move(texCmd));
            } else if (cmd == 0x04) { // BindShader
                // Read null-terminated string
                std::string name;
                while (offset < size && data[offset] != 0) {
                    name.push_back((char)data[offset]);
                    ++offset;
                }
                if (offset < size && data[offset] == 0) ++offset;
                auto shdCmd = std::make_unique<GxmBindShaderCommand>();
                shdCmd->shaderName = name;
                printf("[GXM Parse] BindShader: name=%s\n", name.c_str());
                outBuf.add(std::move(shdCmd));
            } else if (cmd == 0x05) { // SetState
                if (offset + 8 > size) break;
                uint32_t stateId = *reinterpret_cast<const uint32_t*>(data + offset);
                uint32_t value = *reinterpret_cast<const uint32_t*>(data + offset + 4);
                offset += 8;
                auto stateCmd = std::make_unique<GxmSetStateCommand>();
                stateCmd->stateId = stateId;
                stateCmd->value = value;
                printf("[GXM Parse] SetState: id=%u, value=%u\n", stateId, value);
                outBuf.add(std::move(stateCmd));
            } else if (cmd == 0xFF) {
                printf("[GXM Parse] End of buffer\n");
                break;
            } else {
                printf("[GXM Parse] Unknown command: 0x%X\n", cmd);
                break;
            }
        }
    }

    void beginFrame() {
        // Create a new command buffer for this frame
        currentBuffer = std::make_unique<GxmCommandBuffer>();
        // Get the real (or dummy) GXM command stream
        const uint8_t* data = nullptr;
        size_t size = 0;
        getCurrentGxmCommandBuffer(data, size);
        parseGxmCommandStream(data, size, *currentBuffer);
    }

    void endFrame() {
        // Push the buffer to the queue and execute
        if (currentBuffer) {
            queue.push(std::move(currentBuffer));
        }
        if (!queue.empty()) {
            auto buf = queue.pop();
            if (buf) executor.execute(*buf);
        }
    }

    void clear() { queue.clear(); }

private:
    GxmCommandQueue queue;
    GxmExecutor executor;
    std::unique_ptr<GxmCommandBuffer> currentBuffer;
}; 