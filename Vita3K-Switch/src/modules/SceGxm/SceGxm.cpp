#include "SceGxm.h"
#include "../../renderer/gl/switch_renderer.h"
#include "../../gpu/GxmCommandBuffer.h"
#include "../../core/gpu/GpuSubsystem.h"
#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <chrono>
#include <ctime>
#include <sstream>
#include "../../core/emulator/emulator.h"
#include <tuple>

// Moved from header: validateShaderFormat definition
bool validateShaderFormat(const void* data, uint32_t size) {
    if (!data || size < MIN_SHADER_SIZE) {
        return false;
    }
    // Check shader header
    const uint32_t* header = (const uint32_t*)data;
    if (header[0] != SCE_GXM_SHADER_MAGIC) {
        return false;
    }
    // Check shader version
    if (header[1] != SCE_GXM_SHADER_VERSION) {
        return false;
    }
    // Check parameter count
    uint32_t paramCount = header[2];
    if (paramCount > MAX_SHADER_PARAMETERS) {
        return false;
    }
    // Check attribute count
    uint32_t attrCount = header[3];
    if (attrCount > MAX_SHADER_ATTRIBUTES) {
        return false;
    }
    // Check varying count
    uint32_t varyingCount = header[4];
    if (varyingCount > MAX_SHADER_VARYINGS) {
        return false;
    }
    // Check shader code size
    uint32_t codeSize = size - sizeof(SceGxmProgram);
    if (codeSize < MIN_SHADER_CODE_SIZE || codeSize > MAX_SHADER_CODE_SIZE) {
        return false;
    }
    // Check shader code alignment
    if (codeSize % sizeof(uint32_t) != 0) {
        return false;
    }
    return true;
}

// Syscall wrapper functions
int SceGxm::sceGxmInitializeWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 1) {
        printf("[SceGxm] sceGxmInitialize called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    const void* params = (const void*)args[0];
    return instance->sceGxmInitialize(params);
}

int SceGxm::sceGxmDrawWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 5) {
        printf("[SceGxm] sceGxmDraw called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmContext* context = (SceGxmContext*)args[0];
    SceGxmPrimitiveType primitiveType = (SceGxmPrimitiveType)args[1];
    int vertexCount = args[2];
    const void* vertexData = (const void*)args[3];
    uint32_t vertexSize = args[4];
    
    // Disambiguate to the overload with vertex data
    return instance->sceGxmDraw(context, primitiveType, vertexCount, vertexData, vertexSize, nullptr, 0, 0);
}

int SceGxm::sceGxmBeginSceneWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 7) {
        printf("[SceGxm] sceGxmBeginScene called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmContext* context = (SceGxmContext*)args[0];
    uint32_t displayQueueCount = args[1];
    const SceGxmRenderTarget* renderTarget = (const SceGxmRenderTarget*)args[2];
    const void* displayQueueCallback = (const void*)args[3];
    void* displayQueueCallbackData = (void*)args[4];
    const SceGxmColorSurface* colorSurface = (const SceGxmColorSurface*)args[5];
    const SceGxmDepthStencilSurface* depthStencilSurface = (const SceGxmDepthStencilSurface*)args[6];
    
    return instance->sceGxmBeginScene(
        context,
        displayQueueCount,
        renderTarget,
        displayQueueCallback,
        displayQueueCallbackData,
        colorSurface,
        depthStencilSurface
    );
}

int SceGxm::sceGxmTextureSetFormatWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmTextureSetFormat called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmTexture* texture = (SceGxmTexture*)args[0];
    SceGxmTextureFormat format = (SceGxmTextureFormat)args[1];
    
    return instance->sceGxmTextureSetFormat(texture, format);
}

int SceGxm::sceGxmTextureSetWidthWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmTextureSetWidth called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmTexture* texture = (SceGxmTexture*)args[0];
    uint32_t width = args[1];
    
    return instance->sceGxmTextureSetWidth(texture, width);
}

int SceGxm::sceGxmShaderPatcherRegisterProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 3) {
        printf("[SceGxm] sceGxmShaderPatcherRegisterProgram called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmShaderPatcher* shaderPatcher = (SceGxmShaderPatcher*)args[0];
    const SceGxmProgram* program = (const SceGxmProgram*)args[1];
    uint32_t* programId = (uint32_t*)args[2];
    
    return instance->sceGxmShaderPatcherRegisterProgram(shaderPatcher, program, programId);
}

int SceGxm::sceGxmShaderPatcherUnregisterProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmShaderPatcherUnregisterProgram called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmShaderPatcher* shaderPatcher = (SceGxmShaderPatcher*)args[0];
    const SceGxmProgram* program = (const SceGxmProgram*)args[1];
    
    return instance->sceGxmShaderPatcherUnregisterProgram(shaderPatcher, program);
}

int SceGxm::sceGxmShaderPatcherCreateVertexProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 7) {
        printf("[SceGxm] sceGxmShaderPatcherCreateVertexProgram called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmShaderPatcher* shaderPatcher = (SceGxmShaderPatcher*)args[0];
    uint32_t programId = args[1];
    int inputFormat = args[2];
    int vertexStreamMask = args[3];
    const void* vertexCallback = (const void*)args[4];
    void* vertexCallbackData = (void*)args[5];
    SceGxmVertexProgram** vertexProgram = (SceGxmVertexProgram**)args[6];
    
    return instance->sceGxmShaderPatcherCreateVertexProgram(
        shaderPatcher,
        programId,
        inputFormat,
        vertexStreamMask,
        vertexCallback,
        vertexCallbackData,
        vertexProgram
    );
}

int SceGxm::sceGxmShaderPatcherCreateFragmentProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 7) {
        printf("[SceGxm] sceGxmShaderPatcherCreateFragmentProgram called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmShaderPatcher* shaderPatcher = (SceGxmShaderPatcher*)args[0];
    uint32_t programId = args[1];
    int outputFormat = args[2];
    int multisampleMode = args[3];
    const void* blendInfo = (const void*)args[4];
    const SceGxmProgram* vertexProgram = (const SceGxmProgram*)args[5];
    SceGxmFragmentProgram** fragmentProgram = (SceGxmFragmentProgram**)args[6];
    
    return instance->sceGxmShaderPatcherCreateFragmentProgram(
        shaderPatcher,
        programId,
        outputFormat,
        multisampleMode,
        blendInfo,
        vertexProgram,
        fragmentProgram
    );
}

int SceGxm::sceGxmTextureInitLinearWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 6) {
        printf("[SceGxm] sceGxmTextureInitLinear called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmTexture* texture = (SceGxmTexture*)args[0];
    const void* data = (const void*)args[1];
    SceGxmTextureFormat format = (SceGxmTextureFormat)args[2];
    uint32_t width = args[3];
    uint32_t height = args[4];
    uint32_t mipCount = args[5];
    
    return instance->sceGxmTextureInitLinear(
        texture,
        data,
        format,
        width,
        height,
        mipCount
    );
}

int SceGxm::sceGxmCreateRenderTargetWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmCreateRenderTarget called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    const void* params = (const void*)args[0];
    SceGxmRenderTarget** renderTarget = (SceGxmRenderTarget**)args[1];
    
    return instance->sceGxmCreateRenderTarget(params, renderTarget);
}

int SceGxm::sceGxmDestroyRenderTargetWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 1) {
        printf("[SceGxm] sceGxmDestroyRenderTarget called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmRenderTarget* renderTarget = (SceGxmRenderTarget*)args[0];
    
    return instance->sceGxmDestroyRenderTarget(renderTarget);
}

int SceGxm::sceGxmColorSurfaceInitWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 9) {
        printf("[SceGxm] sceGxmColorSurfaceInit called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmColorSurface* surface = (SceGxmColorSurface*)args[0];
    int format = args[1];
    int width = args[2];
    int height = args[3];
    int stride = args[4];
    uint32_t baseAddress = args[5];
    uint32_t pitch = args[6];
    uint32_t mipCount = args[7];
    void* data = (void*)args[8];
    
    return instance->sceGxmColorSurfaceInit(
        surface,
        format,
        width,
        height,
        stride,
        baseAddress,
        pitch,
        mipCount,
        data
    );
}

int SceGxm::sceGxmDepthStencilSurfaceInitWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 6) {
        printf("[SceGxm] sceGxmDepthStencilSurfaceInit called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmDepthStencilSurface* surface = (SceGxmDepthStencilSurface*)args[0];
    int format = args[1];
    int width = args[2];
    uint32_t height = args[3];
    void* colorBuffer = (void*)args[4];
    void* depthStencilBuffer = (void*)args[5];
    
    return instance->sceGxmDepthStencilSurfaceInit(
        surface,
        format,
        width,
        height,
        colorBuffer,
        depthStencilBuffer
    );
}

int SceGxm::sceGxmShaderPatcherCreateWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmShaderPatcherCreate called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    const void* params = (const void*)args[0];
    SceGxmShaderPatcher** shaderPatcher = (SceGxmShaderPatcher**)args[1];
    
    return instance->sceGxmShaderPatcherCreate(params, shaderPatcher);
}

int SceGxm::sceGxmShaderPatcherDestroyWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 1) {
        printf("[SceGxm] sceGxmShaderPatcherDestroy called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmShaderPatcher* shaderPatcher = (SceGxmShaderPatcher*)args[0];
    
    return instance->sceGxmShaderPatcherDestroy(shaderPatcher);
}

int SceGxm::sceGxmTerminateWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    return instance->sceGxmTerminate();
}

int SceGxm::sceGxmCreateContextWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmCreateContext called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    const void* params = (const void*)args[0];
    SceGxmContext** context = (SceGxmContext**)args[1];
    
    return instance->sceGxmCreateContext(params, context);
}

int SceGxm::sceGxmDestroyContextWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 1) {
        printf("[SceGxm] sceGxmDestroyContext called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmContext* context = (SceGxmContext*)args[0];
    
    return instance->sceGxmDestroyContext(context);
}

int SceGxm::sceGxmSetClearColorWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmSetClearColor called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmContext* context = (SceGxmContext*)args[0];
    uint32_t color = args[1];
    
    return instance->sceGxmSetClearColor(context, color);
}

int SceGxm::sceGxmTextureSetDataWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmTextureSetData called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmTexture* texture = (SceGxmTexture*)args[0];
    const void* data = (const void*)args[1];
    
    return instance->sceGxmTextureSetData(texture, data);
}

int SceGxm::sceGxmTextureSetHeightWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmTextureSetHeight called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmTexture* texture = (SceGxmTexture*)args[0];
    uint32_t height = args[1];
    
    return instance->sceGxmTextureSetHeight(texture, height);
}

int SceGxm::sceGxmEndSceneWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 5) {
        printf("[SceGxm] sceGxmEndScene called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmContext* context = (SceGxmContext*)args[0];
    const void* vertexCallback = (const void*)args[1];
    void* vertexCallbackData = (void*)args[2];
    const void* fragmentCallback = (const void*)args[3];
    void* fragmentCallbackData = (void*)args[4];
    
    return instance->sceGxmEndScene(
        context,
        vertexCallback,
        vertexCallbackData,
        fragmentCallback,
        fragmentCallbackData
    );
}

int SceGxm::sceGxmDrawInstancedWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 6) {
        printf("[SceGxm] sceGxmDrawInstanced called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmContext* context = (SceGxmContext*)args[0];
    SceGxmPrimitiveType primitiveType = (SceGxmPrimitiveType)args[1];
    int vertexCount = args[2];
    const void* vertexData = (const void*)args[3];
    uint32_t vertexSize = args[4];
    uint32_t instanceCount = args[5];
    
    return instance->sceGxmDrawInstanced(context, primitiveType, vertexCount, vertexData, vertexSize, instanceCount);
}

int SceGxm::sceGxmSetFragmentProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmSetFragmentProgram called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmContext* context = (SceGxmContext*)args[0];
    const SceGxmFragmentProgram* program = (const SceGxmFragmentProgram*)args[1];
    
    return instance->sceGxmSetFragmentProgram(context, program);
}

int SceGxm::sceGxmSetVertexProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args) {
    auto& moduleManager = emulator.getModuleManager();
    auto instance = std::static_pointer_cast<SceGxm>(moduleManager.findModule("SceGxm"));
    if (!instance) {
        printf("[SceGxm] Failed to get SceGxm instance\n");
        return vita::SCE_GXM_ERROR_INVALID_STATE;
    }
    
    if (args.size() < 2) {
        printf("[SceGxm] sceGxmSetVertexProgram called with insufficient arguments\n");
        return vita::SCE_GXM_ERROR_INVALID_ARGUMENT;
    }
    
    SceGxmContext* context = (SceGxmContext*)args[0];
    const SceGxmVertexProgram* program = (const SceGxmVertexProgram*)args[1];
    
    return instance->sceGxmSetVertexProgram(context, program);
}

int SceGxm::sceGxmDepthStencilSurfaceInit(SceGxmDepthStencilSurface* surface, int depthStencilFormat, int surfaceType, uint32_t strideInPixels, void* depthData, void* stencilData) {
    if (!surface) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // TODO: Implement depth stencil surface initialization
    return vita::SCE_GXM_ERROR_OK;
}

SceGxm::SceGxm(const std::string& name) : Module(name) {
    memoryManager = std::make_unique<GxmMemoryManager>();
    currentContext = nullptr;
    nextContextId = 1;
    
    // Initialize shader cache
    shaderCache.entries.clear();
    shaderCache.lastCleanup = time(nullptr);
    
    // Initialize context pool
    this->contextPool.maxPoolSize = 0;
    this->contextPool.currentPoolSize = 0;
    this->contextPool.activeContexts = 0;
    this->contextPool.idleContexts.clear();
    this->contextPool.poolHits = 0;
    this->contextPool.poolMisses = 0;
    this->contextPool.lastCleanup = time(nullptr);
    
    // Initialize memory tracking
    this->memoryTracking.totalAllocated = 0;
    this->memoryTracking.totalDeallocated = 0;
    this->memoryTracking.currentAllocated = 0;
    this->memoryTracking.peakAllocated = 0;
    this->memoryTracking.allocationCount = 0;
    this->memoryTracking.deallocationCount = 0;
    this->memoryTracking.allocationFailures = 0;
    this->memoryTracking.peakAllocationCount = 0;
    
    // Register syscalls
    registerSyscalls();
}

bool SceGxm::validateShaderProgram(SceGxmProgram* program) {
    if (!program) {
        return false;
    }
    
    // Reset validation state
    program->validationFlags = static_cast<uint32_t>(SceGxmShaderValidationFlags::VALIDATION_NONE);
    program->validationErrors = static_cast<uint32_t>(SceGxmShaderValidationError::ERROR_NONE);
    program->lastValidationError.clear();
    
    // Validate shader size
    if (program->size < MIN_SHADER_SIZE || program->size > MAX_SHADER_SIZE) {
        program->validationErrors |= static_cast<uint32_t>(SceGxmShaderValidationError::ERROR_INVALID_SIZE);
        program->lastValidationError = "Invalid shader size";
        return false;
    }
    program->validationFlags |= static_cast<uint32_t>(SceGxmShaderValidationFlags::VALIDATION_SHADER_SIZE);
    
    // Validate shader type
    if (program->type != SCE_GXM_PROGRAM_TYPE_VERTEX && 
        program->type != SCE_GXM_PROGRAM_TYPE_FRAGMENT) {
        program->validationErrors |= static_cast<uint32_t>(SceGxmShaderValidationError::ERROR_INVALID_TYPE);
        program->lastValidationError = "Invalid shader type";
        return false;
    }
    program->validationFlags |= static_cast<uint32_t>(SceGxmShaderValidationFlags::VALIDATION_SHADER_TYPE);
    
    // Validate shader format
    if (!validateShaderFormat(program->data, program->size)) {
        program->validationErrors |= static_cast<uint32_t>(SceGxmShaderValidationError::ERROR_INVALID_FORMAT);
        program->lastValidationError = "Invalid shader format";
        return false;
    }
    program->validationFlags |= static_cast<uint32_t>(SceGxmShaderValidationFlags::VALIDATION_SHADER_FORMAT);
    
    // Check for memory leaks
    if (program->memoryUsage > program->peakMemoryUsage) {
        program->validationErrors |= static_cast<uint32_t>(SceGxmShaderValidationError::ERROR_MEMORY_LEAK);
        program->lastValidationError = "Memory leak detected";
        return false;
    }
    
    // Update validation state
    program->lastValidation = time(nullptr);
    program->validationFlags |= static_cast<uint32_t>(SceGxmShaderValidationFlags::VALIDATION_SHADER_VALID);
    
    return true;
}

void SceGxm::cleanupShaderCache() {
    std::lock_guard<std::mutex> lock(shaderCache.cacheMutex);
    
    time_t now = time(nullptr);
    if (now - shaderCache.lastCleanup < CACHE_CLEANUP_INTERVAL) {
        return;
    }
    shaderCache.lastCleanup = now;
    
    // Cleanup old shaders
    for (auto it = shaderCache.entries.begin(); it != shaderCache.entries.end();) {
        if (now - it->second.lastAccess > CACHE_TIMEOUT && 
            it->second.cacheHits < MIN_CACHE_HITS) {
            // Clean up shader
            glDeleteProgram(it->second.program);
            shaderCache.entries.erase(it++);
            
            // Update profiling stats
            this->shaderProfile.memoryUsage -= it->second.memoryUsage;
            this->shaderProfile.cacheMisses++;
        } else {
            ++it;
        }
    }
}

void SceGxm::updateShaderProfile(SceGxmProgram* program, bool isCacheHit) {
    if (!program) return;
    
    // Update shader-specific stats
    program->compileAttempts++;
    program->memoryUsage = memoryManager->getCurrentAllocated();
    program->peakMemoryUsage = std::max(program->peakMemoryUsage, program->memoryUsage);
    
    // Update global shader profile
    this->shaderProfile.totalTime += program->compileTime + program->linkTime;
    this->shaderProfile.drawCalls++;
    this->shaderProfile.memoryUsage = memoryManager->getCurrentAllocated();
    this->shaderProfile.peakMemory = std::max(this->shaderProfile.peakMemory, this->shaderProfile.memoryUsage);
    
    if (isCacheHit) {
        program->cacheHits++;
        this->shaderProfile.cacheHits++;
    } else {
        program->cacheMisses++;
        this->shaderProfile.cacheMisses++;
    }
}

void SceGxm::dumpShaderProfile() {
    printf("[SceGxm] Shader Profile:\n");
    printf("  Total Time: %llu microseconds\n", this->shaderProfile.totalTime);
    printf("  Draw Calls: %llu\n", this->shaderProfile.drawCalls);
    printf("  Shader Switches: %llu\n", this->shaderProfile.shaderSwitches);
    printf("  Cache Hits: %llu\n", this->shaderProfile.cacheHits);
    printf("  Cache Misses: %llu\n", this->shaderProfile.cacheMisses);
    printf("  Memory Usage: %llu bytes\n", this->shaderProfile.memoryUsage);
    printf("  Peak Memory: %llu bytes\n", this->shaderProfile.peakMemory);
    printf("  Max Concurrent Programs: %u\n", this->shaderProfile.maxConcurrentPrograms);
    
    // Print per-program stats (print program ID only, since struct is not available)
    printf("\nPer-Program Statistics:\n");
    for (const auto& entry : shaderCache.entries) {
        printf("  Program ID: %u\n", entry.first);
        // If you have a pointer to SceGxmProgram, print more details here
    }
}

SceGxmContext* SceGxm::acquireContext(const SceGxmContextInitParams& params) {
    std::unique_lock<std::mutex> lock(this->contextPool.poolMutex);
    
    // Look for an idle context in the pool
    for (auto* context : this->contextPool.idleContexts) {
        if (context->state == SceGxmContextState::READY && !context->isPooled) {
            // Update context state
            context->state = SceGxmContextState::INITIALIZED;
            context->isPooled = true;
            context->lastPoolAccess = time(nullptr);
            context->poolRefCount++;
            
            this->contextPool.activeContexts++;
            this->contextPool.poolHits++;
            
            return context;
        }
    }
    
    // No idle context found, create a new one
    this->contextPool.poolMisses++;
    if (this->contextPool.currentPoolSize >= this->contextPool.maxPoolSize) {
        printf("[SceGxm] Context pool full, waiting for idle context\n");
        this->contextPool.poolCondition.wait(lock, [this] {
            return this->contextPool.idleContexts.size() > 0 || this->contextPool.currentPoolSize < this->contextPool.maxPoolSize;
        });
    }
    
    // Create new context
    SceGxmContext* newContext = new SceGxmContext();
    memset(newContext, 0, sizeof(SceGxmContext));
    
    // Initialize context
    newContext->state = SceGxmContextState::INITIALIZED;
    newContext->isPooled = true;
    newContext->lastPoolAccess = time(nullptr);
    newContext->poolRefCount = 1;
    
    // Add to pool
    this->contextPool.idleContexts.push_back(newContext);
    this->contextPool.currentPoolSize++;
    this->contextPool.activeContexts++;
    
    return newContext;
}

void SceGxm::releaseContext(SceGxmContext* context) {
    if (!context || !context->isPooled) return;
    
    std::lock_guard<std::mutex> lock(this->contextPool.poolMutex);
    
    // Update context state
    context->state = SceGxmContextState::READY;
    context->isPooled = true;
    context->lastPoolAccess = time(nullptr);
    
    this->contextPool.activeContexts--;
    
    // Notify waiting threads
    this->contextPool.poolCondition.notify_one();
}

void SceGxm::cleanupContextPool() {
    std::lock_guard<std::mutex> lock(this->contextPool.poolMutex);
    
    // Clean up old contexts
    time_t now = time(nullptr);
    if (now - this->contextPool.lastCleanup < 60) return; // Only cleanup every minute
    this->contextPool.lastCleanup = now;
    
    // Remove contexts that haven't been used in a while
    for (auto it = this->contextPool.idleContexts.begin(); it != this->contextPool.idleContexts.end();) {
        if ((*it)->state == SceGxmContextState::READY &&
            (*it)->poolRefCount == 0 &&
            now - (*it)->lastPoolAccess > 300) { // 5 minutes idle
            
            // Clean up context
            delete *it;
            it = this->contextPool.idleContexts.erase(it);
            this->contextPool.currentPoolSize--;
        } else {
            ++it;
        }
    }
}

void SceGxm::dumpPoolStats() {
    printf("[SceGxm] Context Pool Statistics:\n");
    printf("  Max Pool Size: %u\n", this->contextPool.maxPoolSize);
    printf("  Current Size: %u\n", this->contextPool.currentPoolSize);
    printf("  Active Contexts: %u\n", this->contextPool.activeContexts);
    printf("  Idle Contexts: %u\n", this->contextPool.idleContexts.size());
    printf("  Pool Hits: %u\n", this->contextPool.poolHits);
    printf("  Pool Misses: %u\n", this->contextPool.poolMisses);
    printf("  Last Cleanup: %lu seconds ago\n", time(nullptr) - this->contextPool.lastCleanup);
}

void SceGxm::dumpMemoryStats() {
    printf("[SceGxm] Memory Statistics:\n");
    printf("  Total Allocated: %llu bytes\n", this->memoryTracking.totalAllocated);
    printf("  Total Deallocated: %llu bytes\n", this->memoryTracking.totalDeallocated);
    printf("  Current Allocated: %llu bytes\n", this->memoryTracking.currentAllocated);
    printf("  Peak Allocated: %llu bytes\n", this->memoryTracking.peakAllocated);
    printf("  Allocation Count: %llu\n", this->memoryTracking.allocationCount);
    printf("  Deallocation Count: %llu\n", this->memoryTracking.deallocationCount);
    printf("  Allocation Failures: %llu\n", this->memoryTracking.allocationFailures);
    printf("  Peak Allocation Count: %llu\n", this->memoryTracking.peakAllocationCount);
    
    printf("\nMemory Allocation Details:\n");
    for (const auto& alloc : this->memoryTracking.allocations) {
        printf("  Address: %p, Size: %zu bytes\n", alloc.first, alloc.second);
    }
}

int SceGxm::sceGxmInitialize(const void* params) {
    if (!params) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Parse initialization parameters
    const SceGxmInitParams* initParams = (const SceGxmInitParams*)params;
    
    // Initialize context pool
    this->contextPool.maxPoolSize = initParams->maxPoolSize;
    this->contextPool.currentPoolSize = 0;
    this->contextPool.activeContexts = 0;
    this->contextPool.idleContexts.clear();
    this->contextPool.poolHits = 0;
    this->contextPool.poolMisses = 0;
    this->contextPool.lastCleanup = time(nullptr);
    
    // Initialize memory tracking
    this->memoryTracking.totalAllocated = 0;
    this->memoryTracking.totalDeallocated = 0;
    this->memoryTracking.currentAllocated = 0;
    this->memoryTracking.peakAllocated = 0;
    this->memoryTracking.allocationCount = 0;
    this->memoryTracking.deallocationCount = 0;
    this->memoryTracking.allocationFailures = 0;
    this->memoryTracking.peakAllocationCount = 0;
    
    // Initialize shader profile
    this->shaderProfile.totalTime = 0;
    this->shaderProfile.drawCalls = 0;
    this->shaderProfile.shaderSwitches = 0;
    this->shaderProfile.cacheHits = 0;
    this->shaderProfile.cacheMisses = 0;
    this->shaderProfile.memoryUsage = 0;
    this->shaderProfile.peakMemory = 0;
    this->shaderProfile.maxConcurrentPrograms = 0;
    
    return vita::SCE_GXM_ERROR_OK;
}

int SceGxm::sceGxmCreateContext(const void* params, SceGxmContext** context) {
    if (!context) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }

    const SceGxmContextInitParams* initParams = reinterpret_cast<const SceGxmContextInitParams*>(params);
    if (!initParams) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }

    SceGxmContext* newContext = new SceGxmContext();
    if (!newContext) {
        return vita::SCE_GXM_ERROR_OUT_OF_MEMORY;
    }

    newContext->fragmentUsseRingBufferSize = initParams->fragmentUsseRingBufferSize;
    newContext->vertexUsseRingBufferSize = initParams->vertexUsseRingBufferSize;
    newContext->contextId = nextContextId++;
    
    // Add to tracking map
    contexts[newContext] = newContext;
    
    *context = newContext;
    return vita::SCE_GXM_ERROR_OK;
}

bool SceGxm::validateContext(SceGxmContext* context) {
    if (!context) {
        context->lastValidationError = "Null context pointer";
        context->validationErrors++;
        return false;
    }
    
    std::lock_guard<std::mutex> lock(context->stateMutex);
    
    // Check context state
    if (context->state == SceGxmContextState::DESTROYED) {
        context->lastValidationError = "Context is destroyed";
        context->validationErrors++;
        return false;
    }
    
    // Check memory usage
    if (context->memoryStats.currentAllocated > context->memoryStats.peakAllocated) {
        context->lastValidationError = "Memory usage exceeds peak allocation";
        context->validationErrors++;
        return false;
    }
    
    // Check memory leaks
    if (context->stats.memoryLeakCount > 0) {
        context->lastValidationError = "Memory leaks detected";
        context->validationErrors++;
        return false;
    }
    
    // Check buffer sizes
    if (context->hostMemSize < SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE) {
        context->lastValidationError = "Insufficient host memory size";
        context->validationErrors++;
        return false;
    }
    
    return true;
}

int SceGxm::sceGxmSetContext(SceGxmContext* context) {
    if (!context) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }
    if (!validateContext(context)) {
        return vita::SCE_GXM_ERROR_INVALID_CONTEXT;
    }

    // Wait for currently drawing context to finish
    if (currentContext && currentContext->isDrawing) {
        std::unique_lock<std::mutex> lk(currentContext->drawMutex);
        currentContext->drawCondition.wait(lk, [this] { return !currentContext->isDrawing; });
    }

    if (currentContext) {
        std::lock_guard<std::mutex> lk(currentContext->stateMutex);
        currentContext->state = SceGxmContextState::READY;
    }

    context->state = SceGxmContextState::READY;
    context->lastUsed = time(nullptr);
    currentContext = context;
    return vita::SCE_GXM_ERROR_OK;
}

SceGxm::~SceGxm() {
    finalize();
    }

bool SceGxm::initialize(SwitchRenderer* renderer) {
    if (!renderer) {
        return false;
}

    printf("[SceGxm] Initializing GXM module\n");
    
    // Initialize OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    initialized = true;
    printf("[SceGxm] GXM module initialized successfully\n");
    
    return true;
}

void SceGxm::finalize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return;
}

    printf("[SceGxm] Finalizing GXM module\n");
    
    // Clean up resources
    currentContext = nullptr;
        currentRenderTarget = nullptr;
    currentColorSurface = nullptr;
    currentDepthStencilSurface = nullptr;
    currentVertexProgram = nullptr;
    currentFragmentProgram = nullptr;
    
    initialized = false;
    printf("[SceGxm] GXM module finalized\n");
}

// Helper to emit a buffer upload command to the GXM command buffer
void SceGxm::emitUploadVertexBuffer(const void* data, size_t size) {
    // Get the current GXM command buffer (stub: real impl should get from GpuSubsystem)
    auto* gpu = Emulator::getInstance().getGpuSubsystem();
    if (!gpu) return;
    auto cmd = std::make_unique<GxmUploadVertexBufferCommand>();
    cmd->data = malloc(size);
    memcpy((void*)cmd->data, data, size);
    cmd->size = size;
    gpu->getCommandBuffer().add(std::move(cmd));
}
void SceGxm::emitUploadIndexBuffer(const void* data, size_t size, uint32_t indexType) {
    auto* gpu = Emulator::getInstance().getGpuSubsystem();
    if (!gpu) return;
    auto cmd = std::make_unique<GxmUploadIndexBufferCommand>();
    cmd->data = malloc(size);
    memcpy((void*)cmd->data, data, size);
    cmd->size = size;
    cmd->indexType = indexType;
    gpu->getCommandBuffer().add(std::move(cmd));
}
void SceGxm::emitUploadShader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc) {
    auto* gpu = Emulator::getInstance().getGpuSubsystem();
    if (!gpu) return;
    auto cmd = std::make_unique<GxmUploadShaderCommand>();
    cmd->name = name;
    cmd->vertSrc = vertSrc;
    cmd->fragSrc = fragSrc;
    gpu->getCommandBuffer().add(std::move(cmd));
}

// In sceGxmDraw, extract vertex/index data from emulated memory and emit upload commands
int SceGxm::sceGxmDraw(SceGxmContext* context, SceGxmPrimitiveType primType, int vertexCount, const void* vertexData, uint32_t vertexSize, const void* indexData, uint32_t indexCount, uint32_t indexType) {
    printf("[SceGxm] sceGxmDraw called: primType=%d, vertexCount=%d, indexCount=%d\n", primType, vertexCount, indexCount);
    if (!context) return vita::SCE_GXM_ERROR_INVALID_POINTER;
    // --- Automated attribute/uniform/sampler extraction (stub: use dummy data) ---
    // Example: position (vec3, offset 0), texcoord (vec2, offset 12), stride 20
    std::vector<std::tuple<int, int, GLenum, size_t>> layout = { {0, 3, GL_FLOAT, 0}, {1, 2, GL_FLOAT, 12} };
    emitSetupVertexAttributes(layout, 20);
    // Example: set uniform 'uColor' to (1,1,1,1)
    emitSetUniform("uColor", {1.0f, 1.0f, 1.0f, 1.0f});
    // Example: set sampler 'uTexture' to unit 0
    emitSetSampler("uTexture", 0);
    // --- End automated extraction stub ---
    // Extract vertex data from emulated memory
    if (vertexData && vertexCount > 0 && vertexSize > 0) {
        emitUploadVertexBuffer(vertexData, vertexCount * vertexSize);
    }
    // Extract index data if present
    if (indexData && indexCount > 0) {
        size_t indexSize = (indexType == 0) ? sizeof(uint16_t) : sizeof(uint32_t);
        emitUploadIndexBuffer(indexData, indexCount * indexSize, indexType);
    }
    // TODO: Emit draw command (already handled by GpuSubsystem)
    return 0;
}
// In sceGxmShaderPatcherRegisterProgram, translate shader and emit upload command
int SceGxm::sceGxmShaderPatcherRegisterProgram(SceGxmShaderPatcher* shaderPatcher, const SceGxmProgram* programHeader, uint32_t* programId) {
    printf("[SceGxm] sceGxmShaderPatcherRegisterProgram called\n");
    if (!shaderPatcher || !programHeader || !programId) return vita::SCE_GXM_ERROR_INVALID_POINTER;
    // Translate shader
    std::string vertSrc, fragSrc;
    if (programHeader->type == 0) vertSrc = translateShader(programHeader, programHeader->programSize, true);
    else fragSrc = translateShader(programHeader, programHeader->programSize, false);
    // Use programId as name for now
    std::string name = std::to_string(*programId);
    emitUploadShader(name, vertSrc, fragSrc);
    *programId = 1; // Dummy
    return 0;
}

void SceGxm::emitSetupVertexAttributes(const std::vector<std::tuple<int, int, GLenum, size_t>>& layout, size_t stride) {
    auto* gpu = Emulator::getInstance().getGpuSubsystem();
    if (!gpu) return;
    auto cmd = std::make_unique<GxmSetupVertexAttributesCommand>();
    cmd->layout = layout;
    cmd->stride = stride;
    gpu->getCommandBuffer().add(std::move(cmd));
}
void SceGxm::emitSetUniform(const std::string& name, const std::vector<float>& values) {
    auto* gpu = Emulator::getInstance().getGpuSubsystem();
    if (!gpu) return;
    auto cmd = std::make_unique<GxmSetUniformCommand>();
    cmd->name = name;
    cmd->values = values;
    gpu->getCommandBuffer().add(std::move(cmd));
}
void SceGxm::emitSetSampler(const std::string& name, int unit) {
    auto* gpu = Emulator::getInstance().getGpuSubsystem();
    if (!gpu) return;
    auto cmd = std::make_unique<GxmSetSamplerCommand>();
    cmd->name = name;
    cmd->unit = unit;
    gpu->getCommandBuffer().add(std::move(cmd));
}
