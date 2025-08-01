#include "RendererSoftware.h"
#include <iostream>
#include <string>
#include <cstring>

bool RendererSoftware::create_context() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("VitaNS Software", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }
    sdlRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        window = nullptr;
        return false;
    }
    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
    return true;
}

void RendererSoftware::destroy_context() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if (sdlRenderer) {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void RendererSoftware::create_texture(int w, int h) {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    texture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (!texture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
        return;
    }
    tex_width = w;
    tex_height = h;
}

void RendererSoftware::upload_framebuffer(const uint8_t* pixels, int w, int h) {
    if (!texture || tex_width != w || tex_height != h) {
        create_texture(w, h);
    }
    if (texture) {
        void* pixels_ptr;
        int pitch;
        SDL_LockTexture(texture, nullptr, &pixels_ptr, &pitch);
        memcpy(pixels_ptr, pixels, w * h * 4);
        SDL_UnlockTexture(texture);
    }
}

bool RendererSoftware::init() {
    if (!create_context()) return false;
    return true;
}

void RendererSoftware::draw_frame(/* const FrameData& frame */) {
    if (texture) {
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, texture, nullptr, nullptr);
    }
}

void RendererSoftware::present() {
    SDL_RenderPresent(sdlRenderer);
}

void RendererSoftware::shutdown() {
    destroy_context();
}

void RendererSoftware::beginFrame() {
    // TODO: Add logic if needed
}

void RendererSoftware::endFrame() {
    // TODO: Add logic if needed
}

RendererSoftware::~RendererSoftware() {
    shutdown();
} 