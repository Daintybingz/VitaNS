#include "Renderer.h"
#include <iostream>

class RendererStub : public Renderer {
public:
    bool init() override {
        std::cout << "RendererStub: init() called" << std::endl;
        return true;
    }
    void draw_frame(/* const FrameData& frame */) override {
        std::cout << "RendererStub: draw_frame() called" << std::endl;
    }
    void present() override {
        std::cout << "RendererStub: present() called" << std::endl;
    }
    void shutdown() override {
        std::cout << "RendererStub: shutdown() called" << std::endl;
    }
};

// Example usage (to be integrated into emulator core):
// Renderer* renderer = new RendererStub();
// renderer->init();
// renderer->draw_frame();
// renderer->present();
// renderer->shutdown();
// delete renderer; 