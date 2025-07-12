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
    
    // Initialize ImGui SDL2 backend
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
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Main render loop
    bool done = false;
    SDL_Event event;
    
    spdlog::info("[Step 5] Starting main render loop");
    
    while (!done) {
        // Poll and handle events
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }
        
        // Start the ImGui frame
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        // Create a simple ImGui window
        ImGui::Begin("VitaNS Test");
        ImGui::Text("Hello, VitaNS!");
        ImGui::Text("ImGui is working on Nintendo Switch!");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        if (ImGui::Button("Close")) {
            done = true;
        }
        ImGui::End();
        
        // Rendering
        SDL_RenderSetScale(renderer, ImGui::GetIO().DisplayFramebufferScale.x, ImGui::GetIO().DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(0.45f * 255), (Uint8)(0.55f * 255), (Uint8)(0.60f * 255), (Uint8)(255));
        SDL_RenderClear(renderer);
        
        // Render ImGui
        ImGui_ImplSDL2_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(renderer);
        

        
        // Cap at 60 FPS
        SDL_Delay(16);
    }

    // Cleanup ImGui
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    spdlog::info("VitaNS application closed successfully");
    return 0;
}
