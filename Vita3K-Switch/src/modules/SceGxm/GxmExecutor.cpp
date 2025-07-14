#include "GxmExecutor.h"
#include "../../renderer/RendererGLES2.h"
#include "../../renderer/RendererSoftware.h"
#include <iostream>
#include <cstring>

GxmExecutor::GxmExecutor(Renderer* renderer) : renderer(renderer) {
    printf("[GxmExecutor] Initialized with renderer\n");
}

GxmExecutor::~GxmExecutor() {
    printf("[GxmExecutor] Destroyed\n");
}

void GxmExecutor::executeCommand(const GxmCommand& cmd) {
    switch (cmd.type) {
        case GxmCommandType::Draw:
            executeDraw(cmd);
            break;
        case GxmCommandType::Clear:
            executeClear(cmd);
            break;
        case GxmCommandType::BindTexture:
            executeBindTexture(cmd);
            break;
        case GxmCommandType::BindShader:
            executeBindShader(cmd);
            break;
        case GxmCommandType::SetState:
            executeSetState(cmd);
            break;
        case GxmCommandType::UploadVertexBuffer:
            executeUploadVertexBuffer(cmd);
            break;
        case GxmCommandType::UploadIndexBuffer:
            executeUploadIndexBuffer(cmd);
            break;
        case GxmCommandType::UploadShader:
            executeUploadShader(cmd);
            break;
        default:
            printf("[GxmExecutor] Unknown command type: %d\n", static_cast<int>(cmd.type));
            break;
    }
}

void GxmExecutor::parseGxmCommandStream(const uint8_t* data, size_t size) {
    // Parse GXM command buffer and create GxmCommand objects
    // This is a simplified implementation - real GXM parsing would be more complex
    printf("[GxmExecutor] Parsing GXM command stream (%zu bytes)\n", size);
    
    // For now, just create a dummy draw command
    GxmCommand drawCmd;
    drawCmd.type = GxmCommandType::Draw;
    drawCmd.data = {0, 0, 0, 0}; // Dummy data
    
    executeCommand(drawCmd);
}

void GxmExecutor::executeDraw(const GxmCommand& cmd) {
    auto* gles2 = dynamic_cast<RendererGLES2*>(renderer);
    if (gles2) {
        // Execute draw call on GLES2 renderer
        printf("[GxmExecutor] Executing draw call on GLES2 renderer\n");
        // gles2->draw_triangles(...);
    } else {
        auto* software = dynamic_cast<RendererSoftware*>(renderer);
        if (software) {
            // Software renderer doesn't support individual draw calls
            // It only supports framebuffer upload
            printf("[GxmExecutor] Draw call ignored on software renderer\n");
        }
    }
}

void GxmExecutor::executeClear(const GxmCommand& cmd) {
    auto* gles2 = dynamic_cast<RendererGLES2*>(renderer);
    if (gles2) {
        printf("[GxmExecutor] Executing clear on GLES2 renderer\n");
        // gles2->clear(...);
    }
}

void GxmExecutor::executeBindTexture(const GxmCommand& cmd) {
    auto* gles2 = dynamic_cast<RendererGLES2*>(renderer);
    if (gles2) {
        printf("[GxmExecutor] Executing bind texture on GLES2 renderer\n");
        // gles2->bind_texture(...);
    }
}

void GxmExecutor::executeBindShader(const GxmCommand& cmd) {
    auto* gles2 = dynamic_cast<RendererGLES2*>(renderer);
    if (gles2) {
        printf("[GxmExecutor] Executing bind shader on GLES2 renderer\n");
        // gles2->bind_shader(...);
    }
}

void GxmExecutor::executeSetState(const GxmCommand& cmd) {
    auto* gles2 = dynamic_cast<RendererGLES2*>(renderer);
    if (gles2) {
        printf("[GxmExecutor] Executing set state on GLES2 renderer\n");
        // gles2->set_state(...);
    }
}

void GxmExecutor::executeUploadVertexBuffer(const GxmCommand& cmd) {
    auto* gles2 = dynamic_cast<RendererGLES2*>(renderer);
    if (gles2) {
        printf("[GxmExecutor] Executing upload vertex buffer on GLES2 renderer\n");
        // gles2->upload_vertex_buffer(...);
    }
}

void GxmExecutor::executeUploadIndexBuffer(const GxmCommand& cmd) {
    auto* gles2 = dynamic_cast<RendererGLES2*>(renderer);
    if (gles2) {
        printf("[GxmExecutor] Executing upload index buffer on GLES2 renderer\n");
        // gles2->upload_index_buffer(...);
    }
}

void GxmExecutor::executeUploadShader(const GxmCommand& cmd) {
    auto* gles2 = dynamic_cast<RendererGLES2*>(renderer);
    if (gles2) {
        printf("[GxmExecutor] Executing upload shader on GLES2 renderer\n");
        // gles2->upload_shader(...);
    }
} 