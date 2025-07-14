#pragma once
#include "../../renderer/Renderer.h"
#include <vector>
#include <string>

enum class GxmCommandType {
    Draw,
    Clear,
    BindTexture,
    BindShader,
    SetState,
    UploadVertexBuffer,
    UploadIndexBuffer,
    UploadShader
};

struct GxmCommand {
    GxmCommandType type;
    std::vector<uint8_t> data;
};

class GxmExecutor {
public:
    GxmExecutor(Renderer* renderer);
    ~GxmExecutor();
    
    void executeCommand(const GxmCommand& cmd);
    void parseGxmCommandStream(const uint8_t* data, size_t size);
    
private:
    Renderer* renderer;
    void executeDraw(const GxmCommand& cmd);
    void executeClear(const GxmCommand& cmd);
    void executeBindTexture(const GxmCommand& cmd);
    void executeBindShader(const GxmCommand& cmd);
    void executeSetState(const GxmCommand& cmd);
    void executeUploadVertexBuffer(const GxmCommand& cmd);
    void executeUploadIndexBuffer(const GxmCommand& cmd);
    void executeUploadShader(const GxmCommand& cmd);
}; 