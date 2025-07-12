#include <switch.h>
#include <math.h>
#include <cstdio>
#include <string>
#include <filesystem>
#include "renderer/gl/switch_renderer.h"
#include "core/emulator/emulator.h"
#include "core/memory/memory_manager.h"
#include "core/cpu/switch_cpu_backend.h"
#include "filesystem/vita_filesystem.h"
#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "firmware/vita_os.h"
#include <imgui.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "core/fuse_box.h"

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

// Base paths
#define BASE_PATH "sdmc:/switch/vitans"
#define GAME_PATH "sdmc:/switch/vitans/games"

// Global variables
bool running = true;
SwitchRenderer renderer;
Emulator& emulator = Emulator::getInstance();
VitaFileSystem& fileSystem = VitaFileSystem::getInstance();

// Forward declarations
void initialize_paths();
void show_menu();
void plutonium_main_menu();
void imgui_main_menu(SDL_Window* win, SDL_GLContext gl_context);

int main(int argc, char **argv) {
    // Set up spdlog to log to file on SD card
    auto file_logger = spdlog::basic_logger_mt("file_logger", "sdmc:/switch/vitans/log.txt");
    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::debug); // Log everything
    spdlog::info("VitaNS startup");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        spdlog::error("SDL_Init Error: {}", SDL_GetError());
        return 1;
    }
    spdlog::info("SDL initialized successfully");
    SDL_Window* win = SDL_CreateWindow("Vita3K-Switch Minimal GUI", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (!win) {
        spdlog::error("SDL_CreateWindow Error: {}", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    spdlog::info("SDL window created successfully");

    // Initialize console for debug output
    consoleInit(NULL);
    spdlog::info("Console initialized");
    printf("VitaNS - Vita3K for Nintendo Switch\n");
    printf("Initializing...\n");

    // Initialize the renderer (window, OpenGL ES context)
    if (!renderer.initialize("VitaNS", SCREEN_WIDTH, SCREEN_HEIGHT)) {
        spdlog::error("Renderer initialization failed!");
        printf("Renderer initialization failed! Exiting in 5 seconds...\n");
        sleep(5);
        renderer.finalize();
        consoleExit(NULL);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    spdlog::info("Renderer initialized successfully");

    // Initialize paths
    initialize_paths();
    spdlog::info("Paths initialized");

    // Initialize file system
    if (!fileSystem.initialize(BASE_PATH)) {
        spdlog::error("File system initialization failed!");
        printf("File system initialization failed! Exiting in 5 seconds...\n");
        sleep(5);
        renderer.finalize();
        consoleExit(NULL);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    spdlog::info("File system initialized successfully");

    // Initialize emulator
    EmulatorConfig config;
    config.log_cpu = false;
    config.log_mem = false;
    config.log_fs = false;
    config.resolution_scale = 1;
    config.vsync = true;
    config.base_path = BASE_PATH;
    config.game_path = GAME_PATH;

    if (!emulator.initialize(config)) {
        printf("Emulator initialization failed! Exiting in 5 seconds...\n");
        sleep(5);
        renderer.finalize();
        consoleExit(NULL);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    // Connect renderer to emulator
    emulator.setRenderer(&renderer);

    printf("Initialization successful!\n");
    printf("Press + to exit, - to show menu\n");

    SDL_GLContext gl_context;
    gl_context = SDL_GL_CreateContext(win);
    if (!gl_context) {
        spdlog::error("SDL_GL_CreateContext Error: {}", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    spdlog::info("OpenGL context created successfully");

    imgui_main_menu(win, gl_context);

    // Cleanup
    fileSystem.finalize();
    renderer.finalize();
    consoleExit(NULL);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}

void initialize_paths() {
    // Create base directories if they don't exist
    std::filesystem::create_directories(BASE_PATH);
    std::filesystem::create_directories(GAME_PATH);
}

void testSystemCalls() {
    printf("\n===== Testing System Calls =====\n");
    
    // Run the CPU to execute some system calls
    SwitchCPUBackend& cpu = emulator.getCPU();
    int result = cpu.run();
    
    printf("CPU execution result: %d\n", result);
    printf("====================\n\n");
}

void show_menu() {
    // This is a placeholder for a future UI menu
    // For now, just print some info to the console
    printf("\n===== VitaNS Menu =====\n");
    printf("Emulator State: %d\n", (int)emulator.getState());
    
    // Show firmware info
    printf("Firmware: [version info unavailable]\n");
    
    if (!emulator.getGameTitle().empty()) {
        printf("Game: %s (%s)\n", 
            emulator.getGameTitle().c_str(), 
            emulator.getGameID().c_str());
    } else {
        printf("No game loaded\n");
    }
    
    // Show menu options
    printf("\nOptions:\n");
    printf("1. Install firmware\n");
    printf("2. Load VPK file\n");
    printf("3. Install VPK file\n");
    printf("4. Run loaded game\n");
    printf("5. Test system calls\n");
    printf("6. Exit\n");
    
    // Get user input
    printf("\nEnter option (1-6): ");
    int option = 0;
    scanf("%d", &option);
    
    // Process option
    switch (option) {
        case 1: {
            // Install firmware
            char pupPath[256] = {0};
            printf("Enter PUP file path: ");
            scanf("%255s", pupPath);
            
            printf("Installing firmware, please wait...\n");
            if (emulator.installFirmware(pupPath)) {
                printf("Firmware installed successfully!\n");
            } else {
                printf("Firmware installation failed!\n");
            }
            break;
        }
        case 2: {
            // Load VPK file
            char vpkPath[256] = {0};
            printf("Enter VPK file path: ");
            scanf("%255s", vpkPath);
            
            if (emulator.loadGame(vpkPath)) {
                printf("VPK loaded successfully!\n");
            } else {
                printf("Failed to load VPK!\n");
            }
            break;
        }
        case 3: {
            // Install VPK file
            char vpkPath[256] = {0};
            printf("Enter VPK file path: ");
            scanf("%255s", vpkPath);
            
            if (emulator.installGame(vpkPath)) {
                printf("VPK installed successfully!\n");
            } else {
                printf("Failed to install VPK!\n");
            }
            break;
        }
        case 4: {
            // Run loaded game
            // TODO: Replace with correct game loaded check if available
            if (emulator.getState() == EmulatorState::RUNNING) {
                if (emulator.runGame()) {
                    printf("Game started successfully!\n");
                } else {
                    printf("Failed to start game!\n");
                }
            } else {
                printf("No game loaded!\n");
            }
            break;
        }
        case 5: {
            // Test system calls
            testSystemCalls();
            break;
        }
        case 6: {
            // Exit
            running = false;
            break;
        }
        default: {
            printf("Invalid option!\n");
            break;
        }
    }
    
    printf("====================\n\n");
}

// ImGui-based main menu and fuse box
void imgui_main_menu(SDL_Window* win, SDL_GLContext gl_context) {
    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(win, gl_context);
    // Remove OpenGL3 initialization for Switch compatibility
    // ImGui_ImplOpenGL3_Init("#version 300 es");

    static char pupPath[256] = "sdmc:/switch/vitans/PSVUPDAT.PUP";
    static char vpkPath[256] = "sdmc:/switch/vitans/games/";
    static bool showFuseBox = false;
    static bool showAbout = false;
    static std::string selectedGamePath;
    bool running = true;
    Emulator& emulator = Emulator::getInstance();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
        }
        // Remove OpenGL3 frame for Switch compatibility
        // ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("VitaNS Main Menu");
        if (ImGui::Button("Install Firmware from PUP")) {
            if (!FuseBox::enableFirmware) {
                ImGui::OpenPopup("Firmware Disabled");
            } else {
                if (firmware::install_firmware_from_pup(pupPath)) {
                    ImGui::OpenPopup("Firmware Success");
                } else {
                    ImGui::OpenPopup("Firmware Failed");
                }
            }
        }
        ImGui::InputText("PUP Path", pupPath, sizeof(pupPath));
        if (ImGui::Button("Browse for Game (VPK)")) {
            // For now, just use the input field
            selectedGamePath = vpkPath;
        }
        ImGui::InputText("VPK Path", vpkPath, sizeof(vpkPath));
        if (ImGui::Button("Run Selected Game")) {
            if (!FuseBox::enableGameLoading) {
                ImGui::OpenPopup("Game Disabled");
            } else if (selectedGamePath.empty()) {
                ImGui::OpenPopup("No Game Selected");
            } else if (!emulator.loadGame(selectedGamePath)) {
                ImGui::OpenPopup("Game Load Failed");
            } else if (!emulator.runGame()) {
                ImGui::OpenPopup("Game Run Failed");
            } else {
                ImGui::OpenPopup("Game Started");
            }
        }
        if (ImGui::Button("Settings / Fuse Box")) {
            showFuseBox = true;
        }
        if (ImGui::Button("About / Help")) {
            showAbout = true;
        }
        if (ImGui::Button("Exit")) {
            running = false;
        }
        ImGui::End();

        // Fuse Box window
        if (showFuseBox) {
            ImGui::Begin("Fuse Box", &showFuseBox);
            ImGui::Checkbox("Enable Renderer (restart required)", &FuseBox::enableRenderer);
            ImGui::Checkbox("Enable Audio (restart required)", &FuseBox::enableAudio);
            ImGui::Checkbox("Enable Input (restart required)", &FuseBox::enableInput);
            ImGui::Checkbox("Enable File System (restart required)", &FuseBox::enableFileSystem);
            ImGui::Checkbox("Enable Logging (restart required)", &FuseBox::enableLogging);
            ImGui::Checkbox("Enable Firmware Install", &FuseBox::enableFirmware);
            ImGui::Checkbox("Enable Game Loading/Running", &FuseBox::enableGameLoading);
            ImGui::Checkbox("Enable About Page", &FuseBox::enableAboutPage);
            ImGui::End();
        }
        // About window
        if (showAbout && FuseBox::enableAboutPage) {
            ImGui::Begin("About / Help", &showAbout);
            ImGui::Text("VitaNS v0.1 (Switch port of Vita3K)");
            ImGui::Separator();
            ImGui::Text("Original Vita3K Team\nSwitch Port: YourNameHere");
            ImGui::Separator();
            ImGui::Text("Usage Tips:\n- Install firmware from official PUP file.\n- Browse and select a VPK to run.\n- Use Settings for configuration.\n- Log file: sdmc:/switch/vitans/log.txt");
            ImGui::End();
        }
        // Popups for feedback
        if (ImGui::BeginPopup("Firmware Disabled")) { ImGui::Text("Firmware install is disabled by Fuse Box!"); ImGui::EndPopup(); }
        if (ImGui::BeginPopup("Firmware Success")) { ImGui::Text("Firmware installed successfully!"); ImGui::EndPopup(); }
        if (ImGui::BeginPopup("Firmware Failed")) { ImGui::Text("Firmware installation failed!"); ImGui::EndPopup(); }
        if (ImGui::BeginPopup("Game Disabled")) { ImGui::Text("Game loading/running is disabled by Fuse Box!"); ImGui::EndPopup(); }
        if (ImGui::BeginPopup("No Game Selected")) { ImGui::Text("No game selected!"); ImGui::EndPopup(); }
        if (ImGui::BeginPopup("Game Load Failed")) { ImGui::Text("Failed to load game!"); ImGui::EndPopup(); }
        if (ImGui::BeginPopup("Game Run Failed")) { ImGui::Text("Failed to start game!"); ImGui::EndPopup(); }
        if (ImGui::BeginPopup("Game Started")) { ImGui::Text("Game started!"); ImGui::EndPopup(); }

        ImGui::Render();
        SDL_GL_MakeCurrent(win, gl_context);
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        // Remove OpenGL3 rendering for Switch compatibility
        // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(win);
    }
    // Cleanup
    // Remove OpenGL3 shutdown for Switch compatibility
    // ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}
