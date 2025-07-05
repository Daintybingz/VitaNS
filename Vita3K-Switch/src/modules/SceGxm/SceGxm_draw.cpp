#include "SceGxm.h"
#include "../../renderer/gl/switch_renderer.h"
#include <cstdio>
#include <cstring>

// Drawing functions
int SceGxm::sceGxmBeginScene(SceGxmContext* context, uint32_t flags, 
                           const SceGxmRenderTarget* renderTarget, 
                           const void* displayQueueCallback, void* displayQueueCallbackData, 
                           const SceGxmColorSurface* colorSurface, 
                           const SceGxmDepthStencilSurface* depthStencilSurface) {
    printf("[SceGxm] sceGxmBeginScene called\n");
    
    if (!context || !renderTarget || !colorSurface) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Store current surfaces
    currentContext = context;
    currentRenderTarget = const_cast<SceGxmRenderTarget*>(renderTarget);
    currentColorSurface = const_cast<SceGxmColorSurface*>(colorSurface);
    currentDepthStencilSurface = const_cast<SceGxmDepthStencilSurface*>(depthStencilSurface);
    
    // In a real implementation, we would bind the framebuffer and set up the viewport
    glViewport(0, 0, renderTarget->width, renderTarget->height);
    
    // Clear the framebuffer
    GLbitfield clearMask = GL_COLOR_BUFFER_BIT;
    if (depthStencilSurface) {
        clearMask |= GL_DEPTH_BUFFER_BIT;
        if (depthStencilSurface->stencilFormat != 0) {
            clearMask |= GL_STENCIL_BUFFER_BIT;
        }
    }
    glClear(clearMask);
    
    return 0;
}

int SceGxm::sceGxmEndScene(SceGxmContext* context, const void* vertexCallback, void* vertexCallbackData, 
                         const void* fragmentCallback, void* fragmentCallbackData) {
    printf("[SceGxm] sceGxmEndScene called\n");
    
    if (!context) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // In a real implementation, we would flush the command buffer and swap buffers
    glFlush();
    
    // Reset current state
    currentVertexProgram = nullptr;
    currentFragmentProgram = nullptr;
    
    return 0;
}

int SceGxm::sceGxmSetVertexProgram(SceGxmContext* context, const SceGxmVertexProgram* vertexProgram) {
    printf("[SceGxm] sceGxmSetVertexProgram called\n");
    
    if (!context) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Store current vertex program
    currentVertexProgram = const_cast<SceGxmVertexProgram*>(vertexProgram);
    
    // In a real implementation, we would bind the OpenGL vertex shader
    
    return 0;
}

int SceGxm::sceGxmSetFragmentProgram(SceGxmContext* context, const SceGxmFragmentProgram* fragmentProgram) {
    printf("[SceGxm] sceGxmSetFragmentProgram called\n");
    
    if (!context) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Store current fragment program
    currentFragmentProgram = const_cast<SceGxmFragmentProgram*>(fragmentProgram);
    
    // In a real implementation, we would bind the OpenGL fragment shader
    
    return 0;
}

int SceGxm::sceGxmDraw(SceGxmContext* context, SceGxmPrimitiveType primType, 
                     SceGxmIndexFormat indexType, const void* indexData, uint32_t indexCount) {
    printf("[SceGxm] sceGxmDraw called: primType=%d, indexCount=%d\n", primType, indexCount);
    
    if (!context) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Check if we have valid programs
    if (!currentVertexProgram || !currentFragmentProgram) {
        printf("[SceGxm] Error: No vertex or fragment program set\n");
        return vita::SCE_GXM_ERROR_INVALID_VALUE;
    }
    
    // In a real implementation, we would set up vertex attributes and draw
    GLenum glPrimType = convertPrimitiveType(primType);
    
    if (indexData) {
        // Indexed drawing
        GLenum indexGlType = (indexType == 0) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
        glDrawElements(glPrimType, indexCount, indexGlType, indexData);
    } else {
        // Non-indexed drawing
        glDrawArrays(glPrimType, 0, indexCount);
    }
    
    return 0;
}

int SceGxm::sceGxmDrawInstanced(SceGxmContext* context, SceGxmPrimitiveType primType, 
                              SceGxmIndexFormat indexType, const void* indexData, 
                              uint32_t indexCount, uint32_t instanceCount) {
    printf("[SceGxm] sceGxmDrawInstanced called: primType=%d, indexCount=%d, instanceCount=%d\n", 
           primType, indexCount, instanceCount);
    
    if (!context) {
        return vita::SCE_GXM_ERROR_INVALID_POINTER;
    }
    
    // Check if we have valid programs
    if (!currentVertexProgram || !currentFragmentProgram) {
        printf("[SceGxm] Error: No vertex or fragment program set\n");
        return vita::SCE_GXM_ERROR_INVALID_VALUE;
    }
    
    // In a real implementation, we would set up vertex attributes and draw instanced
    GLenum glPrimType = convertPrimitiveType(primType);
    
    if (indexData) {
        // Indexed instanced drawing
        GLenum indexGlType = (indexType == 0) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
        #ifdef GL_ES_VERSION_3_0
        glDrawElementsInstanced(glPrimType, indexCount, indexGlType, indexData, instanceCount);
        #else
            // Fallback: log warning or use non-instanced draw
            // TODO: Implement instanced drawing fallback for Switch
        #endif
    } else {
        // Non-indexed instanced drawing
        #ifdef GL_ES_VERSION_3_0
        glDrawArraysInstanced(glPrimType, 0, indexCount, instanceCount);
        #else
            // Fallback: log warning or use non-instanced draw
            // TODO: Implement instanced drawing fallback for Switch
        #endif
    }
    
    return 0;
}
