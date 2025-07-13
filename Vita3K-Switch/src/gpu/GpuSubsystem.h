#pragma once
#include "GxmCommandQueue.h"
#include "GxmExecutor.h"
#include "../renderer/gl/IGraphicsBackend.h"
#include <memory>

class GpuSubsystem {
public:
    GpuSubsystem(IGraphicsBackend* backend)
        : executor(backend) {}

    // Stub: get the current GXM command buffer from emulated memory
    // TODO: Replace with real memory manager/GXM module integration
    bool getCurrentGxmCommandBuffer(const uint8_t*& data, size_t& size) {
        // For now, use a static dummy buffer with a simple protocol
        static uint32_t dummyWords[] = {
            0x01, 0x1,        // Clear, mask=1
            0x02, 4, 0, 3,    // Draw, primType=4, indexType=0, indexCount=3
            0xFF              // End
        };
        data = reinterpret_cast<const uint8_t*>(dummyWords);
        size = sizeof(dummyWords);
        return true;
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