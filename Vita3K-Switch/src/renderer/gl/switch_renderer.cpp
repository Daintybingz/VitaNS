#include "switch_renderer.h"
#include <cstdio>

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

int SwitchRenderer::getWidth() const { return width; }
int SwitchRenderer::getHeight() const { return height; }
