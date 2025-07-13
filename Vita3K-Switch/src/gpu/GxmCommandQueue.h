#pragma once
#include <queue>
#include <memory>
#include "GxmCommandBuffer.h"

class GxmCommandQueue {
public:
    void push(std::unique_ptr<GxmCommandBuffer> buf) { queue.push(std::move(buf)); }
    std::unique_ptr<GxmCommandBuffer> pop() {
        if (queue.empty()) return nullptr;
        auto buf = std::move(queue.front());
        queue.pop();
        return buf;
    }
    void clear() { while (!queue.empty()) queue.pop(); }
    bool empty() const { return queue.empty(); }
private:
    std::queue<std::unique_ptr<GxmCommandBuffer>> queue;
}; 