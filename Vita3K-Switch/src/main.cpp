#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#ifndef __SWITCH__
#include <imgui.h>
#include "imgui_impl_sdl2.h"
#endif
#include <vector>
#include <string>
#include "core/emulator/emulator.h"
// Remove or fix EmulatorConfig.h if not needed

int main(int argc, char **argv) {
    // EARLY CRASH DEBUGGING: Write a file as soon as main starts
    FILE* early_log = fopen("sdmc:/switch/vitans/early_log.txt", "w");
    if (early_log) {
        fprintf(early_log, "main() started\n");
        fclose(early_log);
    } else {
        FILE* root_log = fopen("sdmc:/early_log.txt", "w");
        if (root_log) {
            fprintf(root_log, "main() started (root)\n");
            fclose(root_log);
        }
    }

    // Step 2: Add Logging (before any ImGui code)
    try {
    auto file_logger = spdlog::basic_logger_mt("file_logger", "sdmc:/switch/vitans/log.txt");
    spdlog::set_default_logger(file_logger);
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("[Step 2] Logging initialized and working! (before ImGui)");
    } catch (const spdlog::spdlog_ex& ex) {
        FILE* log_fail = fopen("sdmc:/switch/vitans/log_fail.txt", "w");
        if (log_fail) {
            fprintf(log_fail, "spdlog init failed: %s\n", ex.what());
            fclose(log_fail);
        }
    }

    // Initialize SDL2 and create a window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        spdlog::error("SDL_Init failed: {}", SDL_GetError());
        return 1;
    }
    
    SDL_Window* win = SDL_CreateWindow("VitaNS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                       1280, 720, SDL_WINDOW_SHOWN);
    if (!win) {
        spdlog::error("SDL_CreateWindow failed: {}", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Create SDL2 renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        spdlog::error("SDL_CreateRenderer failed: {}", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    
    // Initialize SDL_ttf for text rendering
    if (TTF_Init() == -1) {
        spdlog::error("TTF_Init failed: {}", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

        // Test only IMGUI_CHECKVERSION() with logging before and after
#ifndef __SWITCH__
    spdlog::info("[Step 3] Before IMGUI_CHECKVERSION");
    IMGUI_CHECKVERSION();
    spdlog::info("[Step 3] After IMGUI_CHECKVERSION");
    
    // Test ImGui context creation
    spdlog::info("[Step 3] Before ImGui::CreateContext");
    ImGui::CreateContext();
    spdlog::info("[Step 3] After ImGui::CreateContext");
    
    // Test ImGui IO access
    spdlog::info("[Step 3] Before ImGui::GetIO");
    ImGuiIO& io = ImGui::GetIO();
    spdlog::info("[Step 3] After ImGui::GetIO");
    
    // Test ImGui style setup
    spdlog::info("[Step 3] Before ImGui::StyleColorsDark");
    ImGui::StyleColorsDark();
    spdlog::info("[Step 3] After ImGui::StyleColorsDark");
#endif
    
    // Test SDL2 renderer without ImGui first
    spdlog::info("[Step 4] Testing SDL2 renderer without ImGui");
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red background
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    spdlog::info("[Step 4] SDL2 renderer test completed");
    
    // Test SDL2 renderer functions individually
    spdlog::info("[Step 4] Before SDL_SetRenderDrawColor");
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green background
    spdlog::info("[Step 4] After SDL_SetRenderDrawColor");
    
    spdlog::info("[Step 4] Before SDL_RenderClear");
    SDL_RenderClear(renderer);
    spdlog::info("[Step 4] After SDL_RenderClear");
    
    spdlog::info("[Step 4] Before SDL_RenderPresent");
    SDL_RenderPresent(renderer);
    spdlog::info("[Step 4] After SDL_RenderPresent");
    
    // Wait 1 second to see green background
    SDL_Delay(1000);
    
    // Test SDL2 functionality without ImGui
    spdlog::info("[Step 4] Testing SDL2 functionality without ImGui");
    
    // Create Vita3K-style UI with SDL2 primitives
    spdlog::info("[Step 4] Creating Vita3K-style UI");
    
    // Load font for text rendering
    TTF_Font* font = TTF_OpenFont("external/imgui/misc/fonts/Roboto-Medium.ttf", 24);
    if (!font) {
        spdlog::error("TTF_OpenFont failed: {}", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    // Emulator initialization
    EmulatorConfig config;
    config.base_path = "sdmc:/switch/vitans";
    config.game_path = "sdmc:/switch/vitans/games";
    Emulator::getInstance().initialize(config, renderer);

    // UI state
    int selectedButton = -1;
    std::string statusMessage = "Welcome to VitaNS!";
    std::vector<std::string> buttonTexts = {"Load Game", "Settings", "Exit"};
    const int numButtons = 3;

    // Main UI loop
    bool running = true;
    SDL_Event event;
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x, my = event.button.y;
                for (int i = 0; i < numButtons; i++) {
                    SDL_Rect btn = {40, 100 + i * 60, 200, 40};
                    if (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h) {
                        selectedButton = i;
                        if (i == 0) statusMessage = "[Stub] Load Game clicked";
                        else if (i == 1) statusMessage = "[Stub] Settings clicked";
                        else if (i == 2) running = false;
                    }
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_DOWN) {
                    selectedButton = (selectedButton + 1) % numButtons;
                } else if (event.key.keysym.sym == SDLK_UP) {
                    selectedButton = (selectedButton - 1 + numButtons) % numButtons;
                } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    if (selectedButton == 0) statusMessage = "[Stub] Load Game selected";
                    else if (selectedButton == 1) statusMessage = "[Stub] Settings selected";
                    else if (selectedButton == 2) running = false;
        }
            }
        }
        // Controller support (Joy-Con)
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_RIGHT]) selectedButton = (selectedButton + 1) % numButtons;
        if (keystate[SDL_SCANCODE_LEFT]) selectedButton = (selectedButton - 1 + numButtons) % numButtons;

        // Clear screen with dark background
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);
        // Draw Vita3K-style header
        SDL_SetRenderDrawColor(renderer, 0, 120, 215, 255);
        SDL_Rect header_rect = {0, 0, 1280, 60};
        SDL_RenderFillRect(renderer, &header_rect);
        // Draw main content area
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_Rect content_rect = {20, 80, 1240, 640};
        SDL_RenderFillRect(renderer, &content_rect);
        // Draw Vita3K-style buttons with hover/selection effects
        for (int i = 0; i < numButtons; i++) {
            SDL_Rect btn = {40, 100 + i * 60, 200, 40};
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            bool isHovered = (mx >= btn.x && mx <= btn.x + btn.w && my >= btn.y && my <= btn.y + btn.h);
            bool isSelected = (selectedButton == i);
            if (isHovered || isSelected) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            }
            SDL_RenderFillRect(renderer, &btn);
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderDrawRect(renderer, &btn);
            // Render button text
            SDL_Color textColor = {220, 220, 220, 255};
            SDL_Surface* textSurf = TTF_RenderUTF8_Blended(font, buttonTexts[i].c_str(), textColor);
            SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurf);
            int tw = 0, th = 0;
            SDL_QueryTexture(textTex, NULL, NULL, &tw, &th);
            SDL_Rect textRect = {btn.x + (btn.w - tw) / 2, btn.y + (btn.h - th) / 2, tw, th};
            SDL_RenderCopy(renderer, textTex, NULL, &textRect);
            SDL_DestroyTexture(textTex);
            SDL_FreeSurface(textSurf);
        }
        // Draw border
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderDrawRect(renderer, &content_rect);
        // Draw status message at the bottom
        SDL_Color statusColor = {180, 180, 255, 255};
        SDL_Surface* statusSurf = TTF_RenderUTF8_Blended(font, statusMessage.c_str(), statusColor);
        SDL_Texture* statusTex = SDL_CreateTextureFromSurface(renderer, statusSurf);
        int sw = 0, sh = 0;
        SDL_QueryTexture(statusTex, NULL, NULL, &sw, &sh);
        SDL_Rect statusRect = {40, 720 - 40, sw, sh};
        SDL_RenderCopy(renderer, statusTex, NULL, &statusRect);
        SDL_DestroyTexture(statusTex);
        SDL_FreeSurface(statusSurf);

        // --- Emulator frame rendering ---
        Emulator::getInstance().renderFrame();

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }
    // Cleanup
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
