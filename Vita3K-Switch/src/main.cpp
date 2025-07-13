#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <imgui.h>
#include "imgui_impl_sdl2.h"

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
    
    // Main UI loop
    bool running = true;
    SDL_Event event;
    
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
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
        
        // Draw Vita3K-style buttons with hover effects
        SDL_Rect button1 = {40, 100, 200, 40};
        SDL_Rect button2 = {40, 160, 200, 40};
        SDL_Rect button3 = {40, 220, 200, 40};
        
        // Get mouse position for hover effects
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        
        // Draw buttons with hover effects
        SDL_Rect buttons[] = {button1, button2, button3};
        const char* buttonTexts[] = {"Load Game", "Settings", "Exit"};
        
        for (int i = 0; i < 3; i++) {
            bool isHovered = (mouseX >= buttons[i].x && mouseX <= buttons[i].x + buttons[i].w &&
                              mouseY >= buttons[i].y && mouseY <= buttons[i].y + buttons[i].h);
            
            // Button color based on hover state
            if (isHovered) {
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255); // Lighter when hovered
            } else {
                SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255); // Normal color
            }
            
            SDL_RenderFillRect(renderer, &buttons[i]);
            
            // Draw button border
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            SDL_RenderDrawRect(renderer, &buttons[i]);
        }
        
        // Draw border
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
        SDL_RenderDrawRect(renderer, &content_rect);
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }
    
    spdlog::info("[Step 4] Vita3K-style UI completed");
    
    // Wait 1 second before cleanup
    SDL_Delay(1000);

    // Skip problematic cleanup and create stable foundation
    spdlog::info("[Step 5] Skipping cleanup - creating stable foundation");
    
    // Just exit without cleanup for now
    spdlog::info("[Step 5] Application completed successfully (cleanup skipped)");
    return 0;
}
