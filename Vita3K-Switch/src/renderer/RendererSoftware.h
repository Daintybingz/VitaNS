#pragma once
#include "Renderer.h"
#include <SDL2/SDL.h>
#include <vector>
#include <string>

class RendererSoftware : public Renderer {
public:
    bool init() override;
    void draw_frame(/* const FrameData& frame */) override;
    void present() override;
    void shutdown() override;
    ~RendererSoftware() override;

    // Upload framebuffer data (RGBA8888) to the texture
    void upload_framebuffer(const uint8_t* pixels, int w, int h);

private:
    SDL_Window* window = nullptr;
    SDL_Renderer* sdlRenderer = nullptr;
    SDL_Texture* texture = nullptr;
    int width = 1280;
    int height = 720;
    int tex_width = 0;
    int tex_height = 0;
    bool create_context();
    void destroy_context();
    void create_texture(int w, int h);
}; 