#pragma once
#include "../module.h"
#include "../../display/display_buffer.h"
#include "../../renderer/Renderer.h"
#include <cstdint>
#include <memory>

// Forward declarations
class SwitchRenderer;
class IGraphicsBackend;

class SceDisplay : public Module {
public:
    SceDisplay();
    ~SceDisplay() override;
    void registerFunctions() override;
    
    // Initialize the display module with a renderer
    void initialize(Renderer* renderer);
    
    // Finalize the display module
    void finalize();
    
    // Render the current framebuffer
    void render();

    // System call implementations
    static int sceDisplaySetFrameBuf(uint32_t frameBuf, uint32_t sync);
    static int sceDisplayWaitVblankStart();
    static int sceDisplayGetFrameBuf(uint32_t* frameBuf, uint32_t* sync);
    
    // Get the display buffer
    static DisplayBuffer* getDisplayBuffer();

    // Module interface
    const char* getName() const override { return "SceDisplay"; }
    bool load() override;
    void unload() override;

private:
    std::unique_ptr<DisplayBuffer> displayBuffer;
    Renderer* renderer = nullptr;
    static uint32_t currentFrameBuf;
    static uint32_t currentSync;
};
