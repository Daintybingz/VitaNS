#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <string>
#include <vector>

struct GraphicsCapabilities {
    std::string glVersion;
    std::string glRenderer;
    std::string glVendor;
    std::string glExtensions;
    bool has_ES3 = false;
    bool has_MRT = false;
    bool has_UBO = false;
    bool has_instancing = false;
    // Add more as needed
};

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

    // Capabilities
    const GraphicsCapabilities& getCapabilities() const { return capabilities; }
    void probeCapabilities();
    void logCapabilities(const std::string& logPath = "sdmc:/switch/vitans/gpu_caps.txt");

    // TODO: Add methods for shader, texture, and framebuffer management

private:
    SDL_Window* window;
    SDL_GLContext glContext;
    int width;
    int height;
    bool initialized;
    GraphicsCapabilities capabilities;
};
