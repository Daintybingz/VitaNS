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

    // Step 2: Add Logging
    try {
        auto file_logger = spdlog::basic_logger_mt("file_logger", "sdmc:/switch/vitans/log.txt");
        spdlog::set_default_logger(file_logger);
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("[Step 2] Logging initialized and working!");
    } catch (const spdlog::spdlog_ex& ex) {
        FILE* log_fail = fopen("sdmc:/switch/vitans/log_fail.txt", "w");
        if (log_fail) {
            fprintf(log_fail, "spdlog init failed: %s\n", ex.what());
            fclose(log_fail);
        }
    }

    // Initialize SDL2 and create a window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }
    SDL_Window* win = SDL_CreateWindow("VitaNS Minimal UI", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (!win) {
        SDL_Quit();
        return 1;
    }

    // Step 3: Add ImGui UI
    spdlog::info("[Step 3] Initializing ImGui");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(win, nullptr);

    bool running = true;
    Uint32 start = SDL_GetTicks();
    while (running && SDL_GetTicks() - start < 2000) { // Run for 2 seconds
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
        }
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("VitaNS Isolation Test");
        ImGui::Text("ImGui is working!");
        ImGui::End();
        ImGui::Render();
        SDL_UpdateWindowSurface(win); // No renderer, just update surface
    }

    spdlog::info("[Step 3] Shutting down ImGui");
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
