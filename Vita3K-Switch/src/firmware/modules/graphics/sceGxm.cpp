#include "sceGxm.h"
#include <iostream>
#include <cstring>
#include <algorithm>

namespace firmware {
namespace modules {
namespace graphics {

// Global GXM manager instance
SceGxmManager g_gxm_manager;

// SceGxmManager implementation
SceGxmManager::SceGxmManager()
    : initialized(false) {
}

SceGxmManager::~SceGxmManager() {
    if (initialized) {
        terminate();
    }
}

int SceGxmManager::initialize(const SceGxmInitializeParams *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (initialized) {
        std::cerr << "GXM manager is already initialized" << std::endl;
        return SCE_GXM_ERROR_ALREADY_INITIALIZED;
    }
    
    if (!params) {
        std::cerr << "Invalid initialization parameters" << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    std::cout << "Initializing GXM manager" << std::endl;
    
    // Copy initialization parameters
    memcpy(&init_params, params, sizeof(SceGxmInitializeParams));
    
    // Clear maps
    contexts.clear();
    render_targets.clear();
    shader_patchers.clear();
    vertex_programs.clear();
    fragment_programs.clear();
    texture_memory.clear();
    shader_cache.clear();
    
    initialized = true;
    std::cout << "GXM manager initialized successfully" << std::endl;
    return 0;
}

int SceGxmManager::terminate() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "GXM manager is not initialized" << std::endl;
        return SCE_GXM_ERROR_UNINITIALIZED;
    }
    
    std::cout << "Terminating GXM manager" << std::endl;
    
    // Clear maps
    contexts.clear();
    render_targets.clear();
    shader_patchers.clear();
    vertex_programs.clear();
    fragment_programs.clear();
    texture_memory.clear();
    shader_cache.clear();
    
    initialized = false;
    std::cout << "GXM manager terminated successfully" << std::endl;
    return 0;
}

int SceGxmManager::create_context(SceGxmContext **context, const SceGxmContextParams *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "GXM manager is not initialized" << std::endl;
        return SCE_GXM_ERROR_UNINITIALIZED;
    }
    
    if (!context || !params) {
        std::cerr << "Invalid context or parameters pointer" << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Create a new context
    std::unique_ptr<SceGxmContext> new_context = std::make_unique<SceGxmContext>();
    
    // Store the context
    *context = new_context.get();
    contexts[*context] = std::move(new_context);
    
    std::cout << "Created GXM context: " << *context << std::endl;
    return 0;
}

int SceGxmManager::destroy_context(SceGxmContext *context) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "GXM manager is not initialized" << std::endl;
        return SCE_GXM_ERROR_UNINITIALIZED;
    }
    
    if (!context) {
        std::cerr << "Invalid context pointer" << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Find the context
    auto it = contexts.find(context);
    if (it == contexts.end()) {
        std::cerr << "Context not found: " << context << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Remove the context
    contexts.erase(it);
    
    std::cout << "Destroyed GXM context: " << context << std::endl;
    return 0;
}

int SceGxmManager::create_render_target(SceGxmRenderTarget **renderTarget, const SceGxmRenderTargetParams *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "GXM manager is not initialized" << std::endl;
        return SCE_GXM_ERROR_UNINITIALIZED;
    }
    
    if (!renderTarget || !params) {
        std::cerr << "Invalid render target or parameters pointer" << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Create a new render target
    std::unique_ptr<SceGxmRenderTarget> new_target = std::make_unique<SceGxmRenderTarget>();
    
    // Store the render target
    *renderTarget = new_target.get();
    render_targets[*renderTarget] = std::move(new_target);
    
    std::cout << "Created GXM render target: " << *renderTarget << std::endl;
    return 0;
}

int SceGxmManager::destroy_render_target(SceGxmRenderTarget *renderTarget) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "GXM manager is not initialized" << std::endl;
        return SCE_GXM_ERROR_UNINITIALIZED;
    }
    
    if (!renderTarget) {
        std::cerr << "Invalid render target pointer" << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Find the render target
    auto it = render_targets.find(renderTarget);
    if (it == render_targets.end()) {
        std::cerr << "Render target not found: " << renderTarget << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Remove the render target
    render_targets.erase(it);
    
    std::cout << "Destroyed GXM render target: " << renderTarget << std::endl;
    return 0;
}

int SceGxmManager::start_scene(SceGxmContext *context, uint32_t flags, const SceGxmRenderTarget *renderTarget,
                             const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *depthStencil) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "GXM manager is not initialized" << std::endl;
        return SCE_GXM_ERROR_UNINITIALIZED;
    }
    
    if (!context || !renderTarget || !colorSurface) {
        std::cerr << "Invalid context, render target, or color surface pointer" << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Find the context
    auto it = contexts.find(context);
    if (it == contexts.end()) {
        std::cerr << "Context not found: " << context << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Find the render target
    auto rt_it = render_targets.find(const_cast<SceGxmRenderTarget*>(renderTarget));
    if (rt_it == render_targets.end()) {
        std::cerr << "Render target not found: " << renderTarget << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    std::cout << "Started GXM scene for context: " << context << std::endl;
    return 0;
}

int SceGxmManager::end_scene(SceGxmContext *context, const SceGxmNotification *vertexNotification,
                           const SceGxmNotification *fragmentNotification) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "GXM manager is not initialized" << std::endl;
        return SCE_GXM_ERROR_UNINITIALIZED;
    }
    
    if (!context) {
        std::cerr << "Invalid context pointer" << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Find the context
    auto it = contexts.find(context);
    if (it == contexts.end()) {
        std::cerr << "Context not found: " << context << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    std::cout << "Ended GXM scene for context: " << context << std::endl;
    return 0;
}

int SceGxmManager::draw_primitives(SceGxmContext *context, uint32_t primType, uint32_t indexType,
                                 const void *indexData, uint32_t indexCount) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "GXM manager is not initialized" << std::endl;
        return SCE_GXM_ERROR_UNINITIALIZED;
    }
    
    if (!context) {
        std::cerr << "Invalid context pointer" << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Find the context
    auto it = contexts.find(context);
    if (it == contexts.end()) {
        std::cerr << "Context not found: " << context << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Check primitive type
    if (!is_valid_primitive_type(primType)) {
        std::cerr << "Invalid primitive type: " << primType << std::endl;
        return SCE_GXM_ERROR_INVALID_VALUE;
    }
    
    // Check index type
    if (!is_valid_index_type(indexType)) {
        std::cerr << "Invalid index type: " << indexType << std::endl;
        return SCE_GXM_ERROR_INVALID_VALUE;
    }
    
    // Check index data
    if (!indexData && indexCount > 0) {
        std::cerr << "Invalid index data pointer" << std::endl;
        return SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Check index count
    if (indexCount == 0) {
        std::cerr << "Invalid index count: " << indexCount << std::endl;
        return SCE_GXM_ERROR_INVALID_INDEX_COUNT;
    }
    
    std::cout << "Drew " << indexCount << " primitives of type " << primType << " for context: " << context << std::endl;
    return 0;
}

bool SceGxmManager::is_valid_primitive_type(uint32_t primType) {
    switch (primType) {
        case SCE_GXM_PRIMITIVE_TRIANGLES:
        case SCE_GXM_PRIMITIVE_TRIANGLE_STRIP:
        case SCE_GXM_PRIMITIVE_TRIANGLE_FAN:
        case SCE_GXM_PRIMITIVE_LINES:
        case SCE_GXM_PRIMITIVE_LINE_STRIP:
        case SCE_GXM_PRIMITIVE_POINTS:
            return true;
        default:
            return false;
    }
}

bool SceGxmManager::is_valid_index_type(uint32_t indexType) {
    // In a real implementation, this would check valid index types
    // For now, we'll just return true
    return true;
}

bool SceGxmManager::is_valid_texture_format(uint32_t format) {
    switch (format) {
        case SCE_GXM_TEXTURE_FORMAT_U8:
        case SCE_GXM_TEXTURE_FORMAT_U4U4U4U4:
        case SCE_GXM_TEXTURE_FORMAT_U8U8:
        case SCE_GXM_TEXTURE_FORMAT_U5U6U5:
        case SCE_GXM_TEXTURE_FORMAT_U1U5U5U5:
        case SCE_GXM_TEXTURE_FORMAT_U8U8U8:
        case SCE_GXM_TEXTURE_FORMAT_U8U8U8U8:
        case SCE_GXM_TEXTURE_FORMAT_F16:
        case SCE_GXM_TEXTURE_FORMAT_F16F16:
        case SCE_GXM_TEXTURE_FORMAT_F32:
        case SCE_GXM_TEXTURE_FORMAT_F32F32:
        case SCE_GXM_TEXTURE_FORMAT_F32M:
        case SCE_GXM_TEXTURE_FORMAT_X8S8S8U8:
        case SCE_GXM_TEXTURE_FORMAT_X8U24:
        case SCE_GXM_TEXTURE_FORMAT_U32:
        case SCE_GXM_TEXTURE_FORMAT_F16F16F16F16:
        case SCE_GXM_TEXTURE_FORMAT_F32F32F32F32:
        case SCE_GXM_TEXTURE_FORMAT_PVRT2BPP:
        case SCE_GXM_TEXTURE_FORMAT_PVRT4BPP:
        case SCE_GXM_TEXTURE_FORMAT_PVRTII2BPP:
        case SCE_GXM_TEXTURE_FORMAT_PVRTII4BPP:
            return true;
        default:
            return false;
    }
}

// Module function implementations
int sceGxmInitialize(const SceGxmInitializeParams *params) {
    return g_gxm_manager.initialize(params);
}

int sceGxmTerminate() {
    return g_gxm_manager.terminate();
}

int sceGxmCreateContext(SceGxmContext **context, const SceGxmContextParams *params) {
    return g_gxm_manager.create_context(context, params);
}

int sceGxmDestroyContext(SceGxmContext *context) {
    return g_gxm_manager.destroy_context(context);
}

int sceGxmCreateRenderTarget(SceGxmRenderTarget **renderTarget, const SceGxmRenderTargetParams *params) {
    return g_gxm_manager.create_render_target(renderTarget, params);
}

int sceGxmDestroyRenderTarget(SceGxmRenderTarget *renderTarget) {
    return g_gxm_manager.destroy_render_target(renderTarget);
}

int sceGxmBeginScene(SceGxmContext *context, uint32_t flags, const SceGxmRenderTarget *renderTarget,
                   const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *depthStencil) {
    return g_gxm_manager.start_scene(context, flags, renderTarget, colorSurface, depthStencil);
}

int sceGxmEndScene(SceGxmContext *context, const SceGxmNotification *vertexNotification,
                 const SceGxmNotification *fragmentNotification) {
    return g_gxm_manager.end_scene(context, vertexNotification, fragmentNotification);
}

int sceGxmDraw(SceGxmContext *context, uint32_t primType, uint32_t indexType,
             const void *indexData, uint32_t indexCount) {
    return g_gxm_manager.draw_primitives(context, primType, indexType, indexData, indexCount);
}

} // namespace graphics
} // namespace modules
} // namespace firmware
