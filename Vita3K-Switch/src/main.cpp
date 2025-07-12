#include <SDL2/SDL.h>
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
    
    // Now test ImGui initialization
    spdlog::info("[Step 4] Before ImGui_ImplSDL2_InitForSDLRenderer");
    if (!ImGui_ImplSDL2_InitForSDLRenderer(win, renderer)) {
        spdlog::error("ImGui_ImplSDL2_InitForSDLRenderer failed");
        ImGui::DestroyContext();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    spdlog::info("[Step 4] After ImGui_ImplSDL2_InitForSDLRenderer");
    
    // Test basic ImGui functions without rendering
    spdlog::info("[Step 5] Testing basic ImGui functions");
    
    // Test ImGui frame creation
    spdlog::info("[Step 5] Before ImGui_ImplSDL2_NewFrame");
    ImGui_ImplSDL2_NewFrame();
    spdlog::info("[Step 5] After ImGui_ImplSDL2_NewFrame");
    
    spdlog::info("[Step 5] Before ImGui::NewFrame");
    ImGui::NewFrame();
    spdlog::info("[Step 5] After ImGui::NewFrame");
    
    // Test ImGui UI creation (without rendering)
    spdlog::info("[Step 5] Before ImGui::Begin");
    ImGui::Begin("VitaNS Test");
    spdlog::info("[Step 5] After ImGui::Begin");
    
    ImGui::Text("Hello, VitaNS!");
    spdlog::info("[Step 5] After ImGui::Text");
    
    spdlog::info("[Step 5] Before ImGui::End");
    ImGui::End();
    spdlog::info("[Step 5] After ImGui::End");
    
    // Test ImGui render (this might be where it crashes)
    spdlog::info("[Step 5] Before ImGui::Render");
    ImGui::Render();
    spdlog::info("[Step 5] After ImGui::Render");
    
    // Simple SDL2 rendering test
    spdlog::info("[Step 5] Before SDL rendering");
    SDL_SetRenderDrawColor(renderer, (Uint8)(0.45f * 255), (Uint8)(0.55f * 255), (Uint8)(0.60f * 255), (Uint8)(255));
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    spdlog::info("[Step 5] After SDL rendering");
    
    spdlog::info("[Step 5] ImGui functionality test completed successfully");
    
    // Wait 2 seconds to observe window
    SDL_Delay(2000);

    // Cleanup ImGui
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    spdlog::info("VitaNS application closed successfully");
    return 0;
}
