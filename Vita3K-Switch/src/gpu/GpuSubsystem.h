#pragma once
#include "GxmCommandQueue.h"
#include "GxmExecutor.h"
#include "../renderer/gl/IGraphicsBackend.h"
#include <memory>

class GpuSubsystem {
public:
    GpuSubsystem(IGraphicsBackend* backend)
        : executor(backend) {}

    void beginFrame() {
        // Create a new command buffer for this frame
        currentBuffer = std::make_unique<GxmCommandBuffer>();
        // Fill with test commands (stub)
        auto clearCmd = std::make_unique<GxmClearCommand>();
        clearCmd->clearMask = 0x1; // GL_COLOR_BUFFER_BIT
        currentBuffer->add(std::move(clearCmd));
        auto drawCmd = std::make_unique<GxmDrawCall>();
        drawCmd->primType = 4; // GL_TRIANGLES
        drawCmd->indexType = 0;
        drawCmd->indexData = nullptr;
        drawCmd->indexCount = 3;
        currentBuffer->add(std::move(drawCmd));
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