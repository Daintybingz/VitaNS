#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <string>

// Switch-specific OpenGL ES renderer stub
class SwitchRenderer {
public:
    SwitchRenderer();
    ~SwitchRenderer();

    // Initialize the renderer (window, context, etc.)
    bool initialize(const std::string& windowTitle, int width, int height);
    void finalize();

    // Frame management
    void beginFrame();
    void endFrame();

    // Buffer swap
    void swapBuffers();

    // Window info
    int getWidth() const;
    int getHeight() const;

    // TODO: Add methods for shader, texture, and framebuffer management

private:
    SDL_Window* window;
    SDL_GLContext glContext;
    int width;
    int height;
    bool initialized;
};
