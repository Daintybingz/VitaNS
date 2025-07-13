#pragma once
#include "GxmCommandQueue.h"
#include "GxmExecutor.h"
#include "../renderer/gl/IGraphicsBackend.h"
#include <memory>

class GpuSubsystem {
public:
    GpuSubsystem(IGraphicsBackend* backend)
        : executor(backend) {}

    // Parse a GXM command stream (stub for now)
    void parseGxmCommandStream(const uint8_t* data, size_t size, GxmCommandBuffer& outBuf) {
        // TODO: Replace this stub with real parsing of GXM command streams from emulated game memory
        // For now, just add a clear and draw command as a placeholder
        auto clearCmd = std::make_unique<GxmClearCommand>();
        clearCmd->clearMask = 0x1; // GL_COLOR_BUFFER_BIT
        outBuf.add(std::move(clearCmd));
        auto drawCmd = std::make_unique<GxmDrawCall>();
        drawCmd->primType = 4; // GL_TRIANGLES
        drawCmd->indexType = 0;
        drawCmd->indexData = nullptr;
        drawCmd->indexCount = 3;
        outBuf.add(std::move(drawCmd));
    }

    void beginFrame() {
        // Create a new command buffer for this frame
        currentBuffer = std::make_unique<GxmCommandBuffer>();
        // TODO: Get real GXM command stream from emulated game memory
        const uint8_t* dummyData = nullptr;
        size_t dummySize = 0;
        parseGxmCommandStream(dummyData, dummySize, *currentBuffer);
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