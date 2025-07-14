#pragma once
#include "GxmCommandQueue.h"
#include "GxmExecutor.h"
#include "../renderer/RendererGLES2.h"
#include <memory>
#include "../core/emulator/emulator.h"
#include "../../modules/SceGxm/SceGxm.h"

class GpuSubsystem {
public:
    GpuSubsystem(RendererGLES2* renderer, SDL_Renderer* sdlRenderer = nullptr)
        : executor(renderer, sdlRenderer) {}

    void setRenderer(RendererGLES2* renderer) { executor = GxmExecutor(renderer); }

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
            } else if (cmd == 0x06) { // UploadVertexBuffer (stub)
                // Example: [cmd][size][data...]
                if (offset + 4 > size) break;
                uint32_t bufSize = *reinterpret_cast<const uint32_t*>(data + offset);
                offset += 4;
                if (offset + bufSize > size) break;
                auto vbCmd = std::make_unique<GxmUploadVertexBufferCommand>();
                vbCmd->data = data + offset; // In real impl, copy or map from emulated memory
                vbCmd->size = bufSize;
                offset += bufSize;
                printf("[GXM Parse] UploadVertexBuffer: size=%u\n", bufSize);
                outBuf.add(std::move(vbCmd));
            } else if (cmd == 0x07) { // UploadIndexBuffer (stub)
                // Example: [cmd][size][indexType][data...]
                if (offset + 8 > size) break;
                uint32_t bufSize = *reinterpret_cast<const uint32_t*>(data + offset);
                uint32_t indexType = *reinterpret_cast<const uint32_t*>(data + offset + 4);
                offset += 8;
                if (offset + bufSize > size) break;
                auto ibCmd = std::make_unique<GxmUploadIndexBufferCommand>();
                ibCmd->data = data + offset;
                ibCmd->size = bufSize;
                ibCmd->indexType = indexType;
                offset += bufSize;
                printf("[GXM Parse] UploadIndexBuffer: size=%u, indexType=%u\n", bufSize, indexType);
                outBuf.add(std::move(ibCmd));
            } else if (cmd == 0x08) { // UploadShader (stub)
                // Example: [cmd][nameLen][name...][vertLen][vertSrc...][fragLen][fragSrc...]
                if (offset + 4 > size) break;
                uint32_t nameLen = *reinterpret_cast<const uint32_t*>(data + offset);
                offset += 4;
                if (offset + nameLen > size) break;
                std::string name(reinterpret_cast<const char*>(data + offset), nameLen);
                offset += nameLen;
                if (offset + 4 > size) break;
                uint32_t vertLen = *reinterpret_cast<const uint32_t*>(data + offset);
                offset += 4;
                if (offset + vertLen > size) break;
                std::string vertSrc(reinterpret_cast<const char*>(data + offset), vertLen);
                offset += vertLen;
                if (offset + 4 > size) break;
                uint32_t fragLen = *reinterpret_cast<const uint32_t*>(data + offset);
                offset += 4;
                if (offset + fragLen > size) break;
                std::string fragSrc(reinterpret_cast<const char*>(data + offset), fragLen);
                offset += fragLen;
                auto shCmd = std::make_unique<GxmUploadShaderCommand>();
                shCmd->name = name;
                shCmd->vertSrc = vertSrc;
                shCmd->fragSrc = fragSrc;
                printf("[GXM Parse] UploadShader: name=%s\n", name.c_str());
                outBuf.add(std::move(shCmd));
            } else if (cmd == 0x09) { // SetupVertexAttributes (stub)
                // Example: [cmd][stride][count][(index,size,type,offset)...]
                if (offset + 8 > size) break;
                size_t stride = *reinterpret_cast<const size_t*>(data + offset);
                size_t count = *reinterpret_cast<const size_t*>(data + offset + 4);
                offset += 8;
                std::vector<std::tuple<int, int, GLenum, size_t>> layout;
                for (size_t i = 0; i < count; ++i) {
                    if (offset + 16 > size) break;
                    int index = *reinterpret_cast<const int*>(data + offset);
                    int sz = *reinterpret_cast<const int*>(data + offset + 4);
                    GLenum type = *reinterpret_cast<const GLenum*>(data + offset + 8);
                    size_t off = *reinterpret_cast<const size_t*>(data + offset + 12);
                    layout.emplace_back(index, sz, type, off);
                    offset += 16;
                }
                auto attrCmd = std::make_unique<GxmSetupVertexAttributesCommand>();
                attrCmd->layout = layout;
                attrCmd->stride = stride;
                printf("[GXM Parse] SetupVertexAttributes: stride=%zu, count=%zu\n", stride, count);
                outBuf.add(std::move(attrCmd));
            } else if (cmd == 0x0A) { // SetUniform (stub)
                // Example: [cmd][nameLen][name...][count][values...]
                if (offset + 4 > size) break;
                uint32_t nameLen = *reinterpret_cast<const uint32_t*>(data + offset);
                offset += 4;
                if (offset + nameLen > size) break;
                std::string name(reinterpret_cast<const char*>(data + offset), nameLen);
                offset += nameLen;
                if (offset + 4 > size) break;
                uint32_t count = *reinterpret_cast<const uint32_t*>(data + offset);
                offset += 4;
                if (offset + count * 4 > size) break;
                std::vector<float> values(count);
                memcpy(values.data(), data + offset, count * 4);
                offset += count * 4;
                auto uniCmd = std::make_unique<GxmSetUniformCommand>();
                uniCmd->name = name;
                uniCmd->values = values;
                printf("[GXM Parse] SetUniform: %s, count=%u\n", name.c_str(), count);
                outBuf.add(std::move(uniCmd));
            } else if (cmd == 0x0B) { // SetSampler (stub)
                // Example: [cmd][nameLen][name...][unit]
                if (offset + 4 > size) break;
                uint32_t nameLen = *reinterpret_cast<const uint32_t*>(data + offset);
                offset += 4;
                if (offset + nameLen > size) break;
                std::string name(reinterpret_cast<const char*>(data + offset), nameLen);
                offset += nameLen;
                if (offset + 4 > size) break;
                int unit = *reinterpret_cast<const int*>(data + offset);
                offset += 4;
                auto smpCmd = std::make_unique<GxmSetSamplerCommand>();
                smpCmd->name = name;
                smpCmd->unit = unit;
                printf("[GXM Parse] SetSampler: %s = %d\n", name.c_str(), unit);
                outBuf.add(std::move(smpCmd));
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