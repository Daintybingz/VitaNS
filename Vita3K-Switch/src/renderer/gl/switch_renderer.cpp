#include "switch_renderer.h"
#include <cstdio>
#include <fstream>

SwitchRenderer::SwitchRenderer()
    : window(nullptr), glContext(nullptr), width(0), height(0), initialized(false) {}

SwitchRenderer::~SwitchRenderer() {
    finalize();
}

bool SwitchRenderer::initialize(const std::string& windowTitle, int w, int h) {
    if (initialized) return true;

    width = w;
    height = h;

    // Initialize SDL2 video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("[SwitchRenderer] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    // Set OpenGL ES attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Create SDL window
    window = SDL_CreateWindow(
        windowTitle.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        printf("[SwitchRenderer] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    // Create OpenGL ES context
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("[SwitchRenderer] SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        window = nullptr;
        return false;
    }

    // Set swap interval (vsync)
    SDL_GL_SetSwapInterval(1);

    // Set viewport
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    initialized = true;
    printf("[SwitchRenderer] Initialized OpenGL ES renderer (%dx%d)\n", width, height);

    // Probe and log capabilities
    probeCapabilities();
    logCapabilities();
    return true;
}

void SwitchRenderer::finalize() {
    if (glContext) {
        SDL_GL_DeleteContext(glContext);
        glContext = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    initialized = false;
}

void SwitchRenderer::beginFrame() {
    if (!initialized) return;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SwitchRenderer::endFrame() {
    swapBuffers();
}

void SwitchRenderer::swapBuffers() {
    if (window && initialized) {
        SDL_GL_SwapWindow(window);
    }
}

void SwitchRenderer::probeCapabilities() {
    // Must have a valid context
    if (!glContext) return;
    capabilities.glVersion = (const char*)glGetString(GL_VERSION);
    capabilities.glRenderer = (const char*)glGetString(GL_RENDERER);
    capabilities.glVendor = (const char*)glGetString(GL_VENDOR);
    capabilities.glExtensions = (const char*)glGetString(GL_EXTENSIONS);

    // Check for ES 3.0+ support
    std::string version = capabilities.glVersion;
    if (version.find("OpenGL ES 3") != std::string::npos) {
        capabilities.has_ES3 = true;
    }

    // Check for common features
    std::string exts = capabilities.glExtensions;
    capabilities.has_MRT = (exts.find("GL_EXT_draw_buffers") != std::string::npos) || (exts.find("GL_NV_draw_buffers") != std::string::npos);
    capabilities.has_UBO = (exts.find("GL_EXT_uniform_buffer_object") != std::string::npos) || (exts.find("GL_OES_uniform_buffer_object") != std::string::npos);
    capabilities.has_instancing = (exts.find("GL_EXT_draw_instanced") != std::string::npos) || (exts.find("GL_NV_draw_instanced") != std::string::npos);
    // Add more as needed
}

void SwitchRenderer::logCapabilities(const std::string& logPath) {
    std::ofstream log(logPath);
    if (!log.is_open()) return;
    log << "GL_VERSION: " << capabilities.glVersion << "\n";
    log << "GL_RENDERER: " << capabilities.glRenderer << "\n";
    log << "GL_VENDOR: " << capabilities.glVendor << "\n";
    log << "GL_EXTENSIONS: " << capabilities.glExtensions << "\n";
    log << "has_ES3: " << capabilities.has_ES3 << "\n";
    log << "has_MRT: " << capabilities.has_MRT << "\n";
    log << "has_UBO: " << capabilities.has_UBO << "\n";
    log << "has_instancing: " << capabilities.has_instancing << "\n";
    log.close();
}

void SwitchRenderer::createShader() {
    printf("[SwitchRenderer] createShader() stub called\n");
}
void SwitchRenderer::bindTexture() {
    printf("[SwitchRenderer] bindTexture() stub called\n");
}
void SwitchRenderer::setFramebuffer() {
    printf("[SwitchRenderer] setFramebuffer() stub called\n");
}
void SwitchRenderer::draw() {
    printf("[SwitchRenderer] draw() stub called\n");
}

int SwitchRenderer::getWidth() const { return width; }
int SwitchRenderer::getHeight() const { return height; }
