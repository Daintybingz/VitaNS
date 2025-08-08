#include "emulator.h"
#include "../../renderer/RendererGLES2.h"
#include "../../renderer/gl/RendererGLES3.h"
#include "../../renderer/gl/IGraphicsBackend.h"
#include "../module/module_registry.h"
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include "modules/SceDisplay/SceDisplay.h"
#include "modules/SceAudio/SceAudio.h"
#include "modules/SceTouch/SceTouch.h"
#include "../memory/memory_manager.h"
#include "../cpu/switch_cpu_backend.h"
#include "../../modules/module.h"
#include "../../input/switch_input.h"
#include "../../firmware/modules/display/sceDisplay.h"
#include "../../firmware/modules/audio/sceAudio.h"
#include "../../firmware/modules/trophy/sceTrophy.h"
#include "../../firmware/modules/camera/sceCamera.h"
#include "../../firmware/modules/power/scePower.h"
#include "../../firmware/modules/input/sceTouch.h"
#include "../../io/vpk.h"
#include "../../filesystem/vita_filesystem.h"
#include "../gpu/GpuSubsystem.h"
#include "../../renderer/Renderer.h"
#include "../../renderer/RendererStub.cpp"
#include <vector>
#include "../../display/display_buffer.h"
#include "../../renderer/RendererFactory.h"
#include "../renderer/RendererSoftware.h"

#ifndef VITANS_PLATFORM_SWITCH
#warning "VITANS_PLATFORM_SWITCH is not defined for this translation unit. Ensure CMake sets it globally."
#endif

namespace fs = std::filesystem;

// Singleton instance
Emulator& Emulator::getInstance() {
    static Emulator instance;
    return instance;
}

Emulator::Emulator()
    : state(EmulatorState::UNINITIALIZED),
      renderer(nullptr) {
}

Emulator::~Emulator() {
    // finalize(); // Removed, not implemented
}

bool Emulator::initialize(const EmulatorConfig& cfg, SDL_Renderer* sdlRenderer) {
    if (state != EmulatorState::UNINITIALIZED) {
        return true; // Already initialized
    }

    config = cfg;
    
    memory_manager = std::make_unique<MemoryManager>();
    if (!memory_manager->initialize()) {
        printf("[Emulator] Failed to initialize memory manager\n");
        return false;
    }
    
    if (!initializeFileSystem()) {
        printf("[Emulator] Failed to initialize file system\n");
        return false;
    }
    
    module_manager = std::make_unique<ModuleManager>();

    // --- PHASE 4: Automatic renderer selection ---
    printf("[Emulator] Detecting renderer capabilities...\n");
    renderer = RendererFactory::createBestRenderer();
    if (!renderer) {
        printf("[Emulator] Failed to create renderer\n");
        return false;
    }
    
    if (!renderer->init()) {
        printf("[Emulator] Failed to initialize renderer\n");
        return false;
    }
    
    printf("[Emulator] Renderer initialized successfully\n");
    
    // Initialize GPU subsystem with the selected renderer
    gpu_subsystem = std::make_unique<GpuSubsystem>(renderer.get());

    // Initialize core modules
    module_manager->registerModule(std::make_shared<SceDisplay>());
    module_manager->registerModule(std::make_shared<SceAudio>());
    // module_manager->registerModule("SceTrophy", std::make_shared<firmware::modules::trophy::SceTrophyManager>()); // Not implemented yet or needs refactor
    // module_manager->registerModule("SceCamera", std::make_shared<firmware::modules::camera::SceCameraManager>()); // Not implemented yet or needs refactor
    // module_manager->registerModule("ScePower", std::make_shared<firmware::modules::power::ScePowerManager>()); // Not implemented yet or needs refactor
    module_manager->registerModule(std::make_shared<SceTouch>());

    auto displayModule = static_cast<SceDisplay*>(module_manager->findModule("SceDisplay").get());
    if (displayModule) {
        displayModule->initialize(renderer.get());
        printf("[Emulator] Display module initialized successfully\n");
    } else {
        printf("[Emulator] Warning: SceDisplay module not found\n");
    }
    
    auto audioModule = static_cast<SceAudio*>(module_manager->findModule("SceAudio").get());
    if (audioModule) {
        audioModule->initialize();
        printf("[Emulator] Audio module initialized successfully\n");
    } else {
        printf("[Emulator] Warning: SceAudio module not found\n");
    }
    
    cpu = std::make_unique<SwitchCPUBackend>();
    input_system = std::make_unique<SwitchInput>();
    
    // Connect touch module to input system
    auto touchModule = static_cast<SceTouch*>(module_manager->findModule("SceTouch").get());
    if (touchModule && input_system) {
        input_system->setTouchModule(touchModule);
        printf("[Emulator] Connected touch module to input system\n");
    }
    
    state = EmulatorState::INITIALIZED;
    printf("[Emulator] Initialized successfully\n");
    return true;
}

bool Emulator::initializeFileSystem() {
    // Create base directories if they don't exist
    try {
        // Create base path
        if (!config.base_path.empty()) {
            fs::create_directories(config.base_path);
        }
        
        // Create game path
        if (!config.game_path.empty()) {
            fs::create_directories(config.game_path);
        }
        
        // Create other required directories
        std::string saveDataPath = config.base_path + "/savedata";
        std::string tempPath = config.base_path + "/temp";
        
        fs::create_directories(saveDataPath);
        fs::create_directories(tempPath);
        
        printf("[Emulator] File system initialized\n");
        printf("[Emulator] Base path: %s\n", config.base_path.c_str());
        printf("[Emulator] Game path: %s\n", config.game_path.c_str());
        
        return true;
    } catch (const std::exception& e) {
        printf("[Emulator] File system initialization failed: %s\n", e.what());
        return false;
    }
}

bool Emulator::loadGame(const std::string& path) {
    if (state == EmulatorState::UNINITIALIZED) {
        printf("[Emulator] Cannot load game: emulator not initialized\n");
        return false;
    }
    
    // Unload current game if any
    unloadGame();
    
    // Check if file exists
    if (!fs::exists(path)) {
        printf("[Emulator] Game file not found: %s\n", path.c_str());
        return false;
    }
    
    // Check file extension
    std::string ext = fs::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // Handle different file types
    if (ext == ".vpk") {
        return loadVpk(path);
    } else if (fs::is_directory(path)) {
        // Load from extracted directory
        // Check for EBOOT.BIN
        if (!fs::exists(path + "/eboot.bin") && !fs::exists(path + "/EBOOT.BIN")) {
            printf("[Emulator] Not a valid game directory (no EBOOT.BIN found): %s\n", path.c_str());
            return false;
        }
        
        // Load game metadata
        if (!loadGameMetadata(path)) {
            printf("[Emulator] Failed to load game metadata\n");
            return false;
        }
        
        // Set game path
        game_path = path;
        
        // Load game executable
        std::string eboot_path = path + "/eboot.bin";
        if (!fs::exists(eboot_path)) {
            eboot_path = path + "/EBOOT.BIN";
        }

        std::ifstream eboot_file(eboot_path, std::ios::binary | std::ios::ate);
        if (!eboot_file.is_open()) {
            printf("[Emulator] Failed to open eboot.bin at %s\n", eboot_path.c_str());
            return false;
        }

        std::streamsize eboot_size = eboot_file.tellg();
        eboot_file.seekg(0, std::ios::beg);

        // Allocate memory for the executable
        // TODO: Use a more realistic address based on ELF parsing or PS Vita memory layout
        uint32_t load_address = 0x81000000; // Example load address
        uint32_t eboot_mem = memory_manager->allocate(eboot_size, MemoryRegionType::RAM);
        if (eboot_mem == 0) {
            printf("[Emulator] Failed to allocate memory for eboot.bin\n");
            return false;
        }

        std::vector<char> eboot_data(eboot_size);
        if (!eboot_file.read(eboot_data.data(), eboot_size)) {
            printf("[Emulator] Failed to read eboot.bin\n");
            memory_manager->free(eboot_mem);
            return false;
        }

        if (!memory_manager->write_memory(eboot_mem, eboot_data.data(), eboot_size)) {
            printf("[Emulator] Failed to write eboot.bin to emulated memory\n");
            memory_manager->free(eboot_mem);
            return false;
        }

        // Allocate stack
        uint32_t stack_size = 1 * 1024 * 1024; // 1MB stack
        uint32_t stack_bottom = memory_manager->allocate(stack_size, MemoryRegionType::RAM);
        if (stack_bottom == 0) {
            printf("[Emulator] Failed to allocate stack memory\n");
            memory_manager->free(eboot_mem);
            return false;
        }
        uint32_t stack_top = stack_bottom + stack_size;

        // Set up CPU state
        cpu->set_pc(eboot_mem); // PC points to the start of the executable
        cpu->set_sp(stack_top); // SP points to the top of the stack

        printf("[Emulator] Game executable loaded at 0x%08X\n", eboot_mem);

        
        printf("[Emulator] Game loaded: %s\n", game_title.c_str());
        return true;
    } else {
        printf("[Emulator] Unsupported file type: %s\n", path.c_str());
        return false;
    }
}

bool Emulator::loadVpk(const std::string& path) {
    printf("[Emulator] Loading VPK file: %s\n", path.c_str());
    
    // VPK loading not implemented (VPKParser missing)
    printf("[Emulator] VPK loading not implemented.\n");
        return false;
}

bool Emulator::installVpk(const std::string& path) {
    printf("[Emulator] Installing VPK file: %s\n", path.c_str());
    
    // VPK install not implemented (VPKParser missing)
    printf("[Emulator] VPK install not implemented.\n");
        return false;
}

bool Emulator::loadGameMetadata(const std::string& path) {
    // TODO: Implement proper metadata loading from PS Vita game files
    // This is a stub implementation
    
    // Extract filename from path
    fs::path game_path(path);
    std::string filename = game_path.filename().string();
    
    // Set dummy metadata for now
    game_title = filename;
    game_id = "DUMMY00000";
    
    return true;
}

void Emulator::unloadGame() {
    if (state == EmulatorState::RUNNING) {
        stop();
    }
    
    // Clear game data
    game_path.clear();
    game_title.clear();
    game_id.clear();
    
    // TODO: Free game-specific resources
    
    printf("[Emulator] Game unloaded\n");
}

void Emulator::run() {
    if (state != EmulatorState::INITIALIZED && state != EmulatorState::PAUSED) {
        printf("[Emulator] Cannot run: invalid state\n");
        return;
    }
    
    if (game_path.empty()) {
        printf("[Emulator] Cannot run: no game loaded\n");
        return;
    }
    
    state = EmulatorState::RUNNING;
    printf("[Emulator] Running game: %s\n", game_title.c_str());
    
    // Main emulation loop
    while (state == EmulatorState::RUNNING) {
        // Update input system
        if (input_system) {
            input_system->update();
        }
        
        // Run CPU for one frame
        cpu->run();
        
        // Render frame
        renderFrame();
        
        // TODO: Add frame timing/synchronization
    }
}

void Emulator::pause() {
    if (state != EmulatorState::RUNNING) {
        return;
    }
    
    state = EmulatorState::PAUSED;
    printf("[Emulator] Paused\n");
    
    // TODO: Pause CPU execution
}

void Emulator::stop() {
    if (state != EmulatorState::RUNNING && state != EmulatorState::PAUSED) {
        return;
    }
    
    state = EmulatorState::INITIALIZED;
    printf("[Emulator] Stopped\n");
    
    // TODO: Stop CPU execution and reset state
}

void Emulator::step() {
    if (state != EmulatorState::PAUSED) {
        printf("[Emulator] Cannot step: emulator not paused\n");
        return;
    }
    
    // Execute a single CPU instruction
    cpu->step();
}

void Emulator::reset() {
    if (state == EmulatorState::UNINITIALIZED) {
        return;
    }
    
    // Stop execution if running
    if (state == EmulatorState::RUNNING || state == EmulatorState::PAUSED) {
        stop();
    }
    
    // Reload the game if one is loaded
    if (!game_path.empty()) {
        std::string currentGame = game_path;
        unloadGame();
        loadGame(currentGame);
    }
    
    printf("[Emulator] Reset\n");
}

EmulatorState Emulator::getState() const {
    return state;
}

const std::string& Emulator::getGamePath() const {
    return game_path;
}

const std::string& Emulator::getGameTitle() const {
    return game_title;
}

const std::string& Emulator::getGameID() const {
    return game_id;
}

MemoryManager& Emulator::getMemoryManager() {
    return *memory_manager;
}

SwitchCPUBackend& Emulator::getCPU() {
    return *cpu;
}

ModuleManager& Emulator::getModuleManager() {
    return *module_manager;
}

#ifndef VITANS_PLATFORM_SWITCH
SceGxm* Emulator::getSceGxm() {
    auto ptr = module_manager->findModule("SceGxm");
    return ptr ? dynamic_cast<SceGxm*>(ptr.get()) : nullptr;
}
#endif

// int Emulator::executeSystemCall(uint32_t nid, uint32_t threadId, const std::vector<uint32_t>& args) {
    // Execute the system call using the module registry
    // return ModuleRegistry::executeSystemCall(*this, nid, threadId, args); // Function not declared in header, removed.
// }

/**
 * Integration point: Emulator::renderFrame
 *
 * This function is responsible for extracting the current emulated framebuffer from the DisplayBuffer (via the SceDisplay module),
 * reading the framebuffer data from emulated memory, and uploading it to the active renderer backend (RendererGLES2).
 *
 * Assumptions:
 * - The framebuffer is in RGBA8888 (A8B8G8R8) format, matching VITA_DISPLAY_PIXEL_FORMAT_A8B8G8R8.
 * - The framebuffer is stored in a contiguous region of emulated memory, accessible via MemoryManager::read_memory.
 * - The DisplayBuffer provides the framebuffer address, width, height, stride, and pixel format.
 *
 * Modularity:
 * - The framebuffer extraction and upload logic is isolated here, making it easy to adapt for future renderer backends (e.g., Vulkan, software).
 * - To support additional pixel formats, add conversion logic before calling upload_framebuffer().
 * - To support multiple backends, abstract the upload and draw logic behind the Renderer interface.
 */
void Emulator::renderFrame() {
    if (state != EmulatorState::RUNNING) {
        return;
    }
#ifndef VITANS_RENDERER_SOFTWARE
    if (gpu_subsystem) {
        gpu_subsystem->beginFrame();
        gpu_subsystem->endFrame();
    }
    auto* gles2 = dynamic_cast<RendererGLES2*>(renderer.get());
    auto displayModule = static_cast<SceDisplay*>(module_manager->findModule("SceDisplay").get());
    if (displayModule) {
        DisplayBuffer* displayBuffer = displayModule->getDisplayBuffer();
        if (displayBuffer) {
            uint32_t fb_addr = 0, fb_width = 0, fb_height = 0, fb_stride = 0, fb_format = 0;
            if (displayBuffer->getFrameBuffer(&fb_addr, &fb_width, &fb_height, &fb_stride, &fb_format)) {
                if (fb_format == VITA_DISPLAY_PIXEL_FORMAT_A8B8G8R8 && fb_addr && fb_width && fb_height) {
                    std::vector<uint8_t> pixels(fb_width * fb_height * 4);
                    memory_manager->read_memory(fb_addr, pixels.data(), fb_width * fb_height * 4);
                    if (gles2) {
                        gles2->upload_framebuffer(pixels.data(), fb_width, fb_height);
                    }
                }
            }
        }
    }
#else
    // Software renderer path
    auto* software = dynamic_cast<RendererSoftware*>(renderer.get());
    if (software) {
        // Implement software framebuffer upload if needed
    }
#endif
    renderer->draw_frame();
    renderer->present();
}

bool Emulator::saveState(const std::string& filename) {
    if (state == EmulatorState::UNINITIALIZED) {
        printf("[Emulator] Cannot save state: emulator not initialized\n");
        return false;
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        printf("[Emulator] Failed to open state file for writing: %s\n", filename.c_str());
        return false;
    }

    CPUContext context = cpu->save_context();
    file.write(reinterpret_cast<const char*>(&context), sizeof(CPUContext));
    
    if (!file.good()) {
        printf("[Emulator] Failed to write CPU context to state file\n");
        file.close();
        return false;
    }

    file.close();
    printf("[Emulator] State saved to %s\n", filename.c_str());
    return true;
}

bool Emulator::loadState(const std::string& filename) {
    if (state == EmulatorState::UNINITIALIZED) {
        printf("[Emulator] Cannot load state: emulator not initialized\n");
        return false;
    }

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        printf("[Emulator] Failed to open state file for reading: %s\n", filename.c_str());
        return false;
    }

    CPUContext context;
    file.read(reinterpret_cast<char*>(&context), sizeof(CPUContext));

    if (!file.good()) {
        printf("[Emulator] Failed to read CPU context from state file\n");
        file.close();
        return false;
    }

    cpu->load_context(context);

    file.close();
    printf("[Emulator] State loaded from %s\n", filename.c_str());
    return true;
}

bool Emulator::installFirmware(const std::string& path) {
    // Stub implementation
    printf("[Emulator] installFirmware called with path: %s\n", path.c_str());
    return false;
}

bool Emulator::installGame(const std::string& path) {
    // Stub implementation
    printf("[Emulator] installGame called with path: %s\n", path.c_str());
    return false;
}

bool Emulator::runGame() {
    if (state != EmulatorState::INITIALIZED && state != EmulatorState::PAUSED) {
        printf("[Emulator] Cannot run game: emulator not in a runnable state.\n");
        return false;
    }
    if (game_path.empty()) {
        printf("[Emulator] Cannot run game: no game loaded.\n");
        return false;
    }

    run();
    return true;
}

