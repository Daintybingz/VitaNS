#pragma once
#include <cstdint>
#include <memory>
#include <mutex>

// Forward declarations
class SwitchRenderer;

// PS Vita display constants
#define VITA_DISPLAY_WIDTH 960
#define VITA_DISPLAY_HEIGHT 544
#define VITA_DISPLAY_STRIDE 960
#define VITA_DISPLAY_PIXEL_FORMAT_A8B8G8R8 0x00000000

// Display buffer structure (matches PS Vita's)
struct DisplayFrameBuffer {
    uint32_t addr;           // Framebuffer base address
    uint32_t width;          // Framebuffer width
    uint32_t height;         // Framebuffer height
    uint32_t stride;         // Framebuffer stride
    uint32_t pixelFormat;    // Pixel format
};

class DisplayBuffer {
public:
    DisplayBuffer();
    ~DisplayBuffer();

    // Initialize the display buffer
    bool initialize(SwitchRenderer* renderer);
    
    // Finalize the display buffer
    void finalize();
    
    // Set the framebuffer from PS Vita memory
    bool setFrameBuffer(uint32_t addr, uint32_t width, uint32_t height, uint32_t stride, uint32_t pixelFormat);
    
    // Get the current framebuffer info
    bool getFrameBuffer(uint32_t* addr, uint32_t* width, uint32_t* height, uint32_t* stride, uint32_t* pixelFormat);
    
    // Render the current framebuffer to the screen
    void render();
    
    // Wait for VBlank
    void waitVblank();

private:
    std::mutex bufferMutex;
    DisplayFrameBuffer currentBuffer;
    SwitchRenderer* renderer;
    bool initialized;
    
    // OpenGL resources
    uint32_t textureId;
    uint32_t vertexBuffer;
    uint32_t indexBuffer;
    uint32_t shaderProgram;
    
    // Initialize OpenGL resources
    bool initializeGL();
    
    // Update texture from PS Vita memory
    void updateTexture();
};
