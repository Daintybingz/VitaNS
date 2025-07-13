#include "SceDisplay.h"
#include "../module_registry.h"
#include "../../renderer/gl/switch_renderer.h"
#include <cstdio>

// Initialize static members
std::unique_ptr<DisplayBuffer> SceDisplay::displayBuffer = nullptr;
uint32_t SceDisplay::currentFrameBuf = 0;
uint32_t SceDisplay::currentSync = 0;

SceDisplay::SceDisplay() : Module("SceDisplay") {
    // Create the display buffer if it doesn't exist
    if (!displayBuffer) {
        displayBuffer = std::make_unique<DisplayBuffer>();
    }
}

SceDisplay::~SceDisplay() {
    finalize();
}

void SceDisplay::registerFunctions() {
    // Register syscalls using lambdas with the correct signature
    registerFunction("sceDisplaySetFrameBuf", 0x289D82FE,
        [](Emulator&, uint32_t, const std::vector<uint32_t>& args) -> int {
            // args: [frameBuf, sync]
            if (args.size() < 2) return -1;
            return SceDisplay::sceDisplaySetFrameBuf(args[0], args[1]);
        },
        2);

    registerFunction("sceDisplayWaitVblankStart", 0x36CD59E1,
        [](Emulator&, uint32_t, const std::vector<uint32_t>&) -> int {
            return SceDisplay::sceDisplayWaitVblankStart();
        },
        0);

    registerFunction("sceDisplayGetFrameBuf", 0x46F186C3,
        [](Emulator&, uint32_t, const std::vector<uint32_t>& args) -> int {
            // args: [frameBufPtr, syncPtr]
            if (args.size() < 2) return -1;
            return SceDisplay::sceDisplayGetFrameBuf(reinterpret_cast<uint32_t*>(args[0]), reinterpret_cast<uint32_t*>(args[1]));
        },
        2);
}


bool SceDisplay::initialize(IGraphicsBackend* renderer) {
    printf("[SceDisplay] Initializing display module\n");
    
    if (!displayBuffer) {
        printf("[SceDisplay] Error: Display buffer is null\n");
        return false;
    }
    
    // Initialize the display buffer
    if (!displayBuffer->initialize(dynamic_cast<SwitchRenderer*>(renderer))) {
        printf("[SceDisplay] Failed to initialize display buffer\n");
        return false;
    }
    
    printf("[SceDisplay] Display module initialized successfully\n");
    return true;
}

void SceDisplay::finalize() {
    printf("[SceDisplay] Finalizing display module\n");
    
    if (displayBuffer) {
        displayBuffer->finalize();
    }
}

void SceDisplay::render() {
    if (displayBuffer) {
        displayBuffer->render();
    }
}

DisplayBuffer* SceDisplay::getDisplayBuffer() {
    return displayBuffer.get();
}

int SceDisplay::sceDisplaySetFrameBuf(uint32_t frameBuf, uint32_t sync) {
    printf("[SceDisplay] sceDisplaySetFrameBuf called: frameBuf=0x%08X, sync=%u\n", frameBuf, sync);
    
    // Store the current framebuffer address and sync mode
    currentFrameBuf = frameBuf;
    currentSync = sync;
    
    // Update the display buffer
    if (displayBuffer) {
        // In a real implementation, we would get the actual width, height, and stride from the game
        // For now, we'll use the default PS Vita display dimensions
        displayBuffer->setFrameBuffer(frameBuf, VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT, VITA_DISPLAY_STRIDE, VITA_DISPLAY_PIXEL_FORMAT_A8B8G8R8);
    }
    
    return 0;
}

int SceDisplay::sceDisplayWaitVblankStart() {
    printf("[SceDisplay] sceDisplayWaitVblankStart called\n");
    
    // Wait for VBlank
    if (displayBuffer) {
        displayBuffer->waitVblank();
    }
    
    return 0;
}

int SceDisplay::sceDisplayGetFrameBuf(uint32_t* frameBuf, uint32_t* sync) {
    printf("[SceDisplay] sceDisplayGetFrameBuf called\n");
    
    // Return the current framebuffer address and sync mode
    if (frameBuf) *frameBuf = currentFrameBuf;
    if (sync) *sync = currentSync;
    
    return 0;
}
