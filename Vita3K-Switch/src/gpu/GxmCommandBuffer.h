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
    UploadVertexBuffer,
    UploadIndexBuffer,
    UploadShader,
    SetupVertexAttributes,
    SetUniform,
    SetSampler,
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
    uint32_t stateId = 0;
    uint32_t value = 0;
    GxmSetStateCommand() { type = GxmGpuCommandType::SetState; }
};

// Upload vertex buffer command
struct GxmUploadVertexBufferCommand : public GxmGpuCommand {
    const void* data;
    size_t size;
    GxmUploadVertexBufferCommand() { type = GxmGpuCommandType::UploadVertexBuffer; }
};
// Upload index buffer command
struct GxmUploadIndexBufferCommand : public GxmGpuCommand {
    const void* data;
    size_t size;
    uint32_t indexType;
    GxmUploadIndexBufferCommand() { type = GxmGpuCommandType::UploadIndexBuffer; }
};
// Upload shader command
struct GxmUploadShaderCommand : public GxmGpuCommand {
    std::string name;
    std::string vertSrc;
    std::string fragSrc;
    GxmUploadShaderCommand() { type = GxmGpuCommandType::UploadShader; }
};

// Setup vertex attributes command
struct GxmSetupVertexAttributesCommand : public GxmGpuCommand {
    std::vector<std::tuple<int, int, GLenum, size_t>> layout;
    size_t stride;
    GxmSetupVertexAttributesCommand() { type = GxmGpuCommandType::SetupVertexAttributes; }
};
// Set uniform command
struct GxmSetUniformCommand : public GxmGpuCommand {
    std::string name;
    std::vector<float> values;
    GxmSetUniformCommand() { type = GxmGpuCommandType::SetUniform; }
};
// Set sampler command
struct GxmSetSamplerCommand : public GxmGpuCommand {
    std::string name;
    int unit;
    GxmSetSamplerCommand() { type = GxmGpuCommandType::SetSampler; }
};

// The command buffer itself
class GxmCommandBuffer {
public:
    std::vector<std::unique_ptr<GxmGpuCommand>> commands;
    void clear() { commands.clear(); }
    void add(std::unique_ptr<GxmGpuCommand> cmd) { commands.push_back(std::move(cmd)); }
}; 