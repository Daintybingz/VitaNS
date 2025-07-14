#include "RendererFactory.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <iostream>
#include <fstream>

bool RendererFactory::testOpenGLContext() {
    SDL_Window* testWindow = nullptr;
    SDL_GLContext testContext = nullptr;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return false;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
    testWindow = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!testWindow) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return false;
    }
    
    testContext = SDL_GL_CreateContext(testWindow);
    if (!testContext) {
        SDL_DestroyWindow(testWindow);
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return false;
    }
    
    SDL_GL_DeleteContext(testContext);
    SDL_DestroyWindow(testWindow);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    return true;
}

void RendererFactory::logCapabilities(const RendererCapabilities& caps) {
    std::ofstream log("sdmc:/switch/vitans/renderer_caps.txt");
    if (log.is_open()) {
        log << "Renderer Capabilities:\n";
        log << "OpenGL Available: " << (caps.hasOpenGL ? "Yes" : "No") << "\n";
        log << "GLES2 Available: " << (caps.hasGLES2 ? "Yes" : "No") << "\n";
        log << "GLES3 Available: " << (caps.hasGLES3 ? "Yes" : "No") << "\n";
        log << "GL Version: " << caps.glVersion << "\n";
        log << "GL Renderer: " << caps.glRenderer << "\n";
        log << "GL Vendor: " << caps.glVendor << "\n";
        log << "GL Extensions: " << caps.glExtensions << "\n";
        log.close();
    }
}

RendererCapabilities RendererFactory::detectCapabilities() {
    RendererCapabilities caps;
    
    // Test if we can create an OpenGL context
    caps.hasOpenGL = testOpenGLContext();
    
    if (caps.hasOpenGL) {
        // Create a temporary context to query capabilities
        SDL_Window* testWindow = nullptr;
        SDL_GLContext testContext = nullptr;
        
        SDL_Init(SDL_INIT_VIDEO);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        
        testWindow = SDL_CreateWindow("Capability Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if (testWindow) {
            testContext = SDL_GL_CreateContext(testWindow);
            if (testContext) {
                caps.glVersion = (const char*)glGetString(GL_VERSION);
                caps.glRenderer = (const char*)glGetString(GL_RENDERER);
                caps.glVendor = (const char*)glGetString(GL_VENDOR);
                caps.glExtensions = (const char*)glGetString(GL_EXTENSIONS);
                
                // Check for GLES2/GLES3 support
                if (caps.glVersion.find("OpenGL ES 2") != std::string::npos) {
                    caps.hasGLES2 = true;
                }
                if (caps.glVersion.find("OpenGL ES 3") != std::string::npos) {
                    caps.hasGLES3 = true;
                    caps.hasGLES2 = true; // GLES3 implies GLES2
                }
                
                SDL_GL_DeleteContext(testContext);
            }
            SDL_DestroyWindow(testWindow);
        }
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
    
    logCapabilities(caps);
    return caps;
}

RendererType RendererFactory::selectBestRenderer(const RendererCapabilities& caps) {
    if (caps.hasGLES2) {
        std::cout << "Selected GLES2 renderer\n";
        return RendererType::GLES2;
    } else {
        std::cout << "Selected Software renderer (fallback)\n";
        return RendererType::Software;
    }
}

std::unique_ptr<Renderer> RendererFactory::createRenderer(RendererType type) {
    switch (type) {
        case RendererType::GLES2:
            return std::make_unique<RendererGLES2>();
        case RendererType::Software:
            return std::make_unique<RendererSoftware>();
        default:
            std::cerr << "Unknown renderer type, falling back to software\n";
            return std::make_unique<RendererSoftware>();
    }
}

std::unique_ptr<Renderer> RendererFactory::createBestRenderer() {
    auto caps = detectCapabilities();
    auto type = selectBestRenderer(caps);
    return createRenderer(type);
} 