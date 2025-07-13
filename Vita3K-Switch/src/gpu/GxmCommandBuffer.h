#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include <string>

// Enum for command types
enum class GxmGpuCommandType {
    Draw,
    Clear,
    BindTexture,
    BindShader,
    SetState,
    Unknown
};

// Base command struct
struct GxmGpuCommand {
    GxmGpuCommandType type;
    virtual ~GxmGpuCommand() = default;
};

// Draw call command
struct GxmDrawCall : public GxmGpuCommand {
    uint32_t primType;
    uint32_t indexType;
    const void* indexData;
    uint32_t indexCount;
    GxmDrawCall() { type = GxmGpuCommandType::Draw; }
};

// Clear command
struct GxmClearCommand : public GxmGpuCommand {
    uint32_t clearMask;
    GxmClearCommand() { type = GxmGpuCommandType::Clear; }
};

// Bind texture command
struct GxmBindTextureCommand : public GxmGpuCommand {
    uint32_t textureUnit;
    uint32_t textureId;
    GxmBindTextureCommand() { type = GxmGpuCommandType::BindTexture; }
};

// Bind shader command
struct GxmBindShaderCommand : public GxmGpuCommand {
    std::string shaderName;
    GxmBindShaderCommand() { type = GxmGpuCommandType::BindShader; }
};

// Set state command (blend, depth, etc.)
struct GxmSetStateCommand : public GxmGpuCommand {
    // Add fields as needed
    GxmSetStateCommand() { type = GxmGpuCommandType::SetState; }
};

// The command buffer itself
class GxmCommandBuffer {
public:
    std::vector<std::unique_ptr<GxmGpuCommand>> commands;
    void clear() { commands.clear(); }
    void add(std::unique_ptr<GxmGpuCommand> cmd) { commands.push_back(std::move(cmd)); }
}; 