#pragma once
#include "Renderer.h"
#include "RendererGLES2.h"
#include "RendererSoftware.h"
#include <memory>
#include <string>

enum class RendererType {
    GLES2,
    Software,
    Unknown
};

struct RendererCapabilities {
    bool hasOpenGL = false;
    bool hasGLES2 = false;
    bool hasGLES3 = false;
    std::string glVersion;
    std::string glRenderer;
    std::string glVendor;
    std::string glExtensions;
};

class RendererFactory {
public:
    static RendererCapabilities detectCapabilities();
    static RendererType selectBestRenderer(const RendererCapabilities& caps);
    static std::unique_ptr<Renderer> createRenderer(RendererType type);
    static std::unique_ptr<Renderer> createBestRenderer();
    
private:
    static bool testOpenGLContext();
    static void logCapabilities(const RendererCapabilities& caps);
}; 