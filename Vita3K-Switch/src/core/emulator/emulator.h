#pragma once

#include <string>
#include <memory>
#include <vector>
#include <mutex>

// Forward declarations
class MemoryManager;
class SwitchCPUBackend;
#include "../../modules/module.h"
class Renderer; // <-- Add this line
class IGraphicsBackend;
class SwitchInput;
class GpuSubsystem;
#include "../../modules/SceGxm/SceGxm.h"

enum class EmulatorState {
    UNINITIALIZED,
    INITIALIZED,
    RUNNING,
    PAUSED,
    STEPPING
};

struct EmulatorConfig {
    std::string base_path;
    std::string game_path;
    bool log_cpu = false;
    bool log_mem = false;
    bool enable_debugger = false;
    bool enable_cheats = false;
    bool log_fs = false;
    int resolution_scale = 1;
    bool vsync = true;
};

class Emulator {
public:
    /**
     * Save the current CPU state to a file.
     * @param filename Path to the state file.
     * @return True on success, false on failure.
     */


    /**
     * Load the CPU state from a file.
     * @param filename Path to the state file.
     * @return True on success, false on failure.
     */


public:
    static Emulator& getInstance();
    Emulator();
    ~Emulator();

    bool initialize(const EmulatorConfig& cfg, SDL_Renderer* sdlRenderer);
    bool initializeFileSystem();
    bool loadGame(const std::string& path);
    bool loadVpk(const std::string& path);
    bool installVpk(const std::string& path);
    bool loadGameMetadata(const std::string& path);
    void unloadGame();
    void run();
    void stop();
    void pause();
    void resume();
    void step();
    void reset();
    void renderFrame();

    /**
     * @brief Saves the current state of the emulator to a file.
     * 
     * This includes the CPU context and could be extended to include memory state.
     * @param filename The path to the file where the state will be saved.
     * @return true if the state was saved successfully, false otherwise.
     */
    bool saveState(const std::string& filename);

    /**
     * @brief Loads the state of the emulator from a file.
     * 
     * This restores the CPU context and could be extended to restore memory state.
     * @param filename The path to the file from which to load the state.
     * @return true if the state was loaded successfully, false otherwise.
     */
    bool loadState(const std::string& filename);

    /**
     * @brief Saves the current state of the emulator to a file.
     * 
     * This includes the CPU context and could be extended to include memory state.
     * @param filename The path to the file where the state will be saved.
     * @return true if the state was saved successfully, false otherwise.
     */


    /**
     * @brief Loads the state of the emulator from a file.
     * 
     * This restores the CPU context and could be extended to restore memory state.
     * @param filename The path to the file from which to load the state.
     * @return true if the state was loaded successfully, false otherwise.
     */


    // Getters
    const std::string& getGamePath() const;
    const std::string& getGameTitle() const;
    const std::string& getGameID() const;
    MemoryManager& getMemoryManager();
    SwitchCPUBackend& getCPU();
    ModuleManager& getModuleManager();
    // GpuSubsystem* getGpu() { return gpu.get(); } // PHASE 1: Commented out
    SceGxm* getSceGxm();

    // Add missing methods
    // void setRenderer(IGraphicsBackend* r); // PHASE 1: Commented out
    EmulatorState getState() const;
    bool installFirmware(const std::string& path);
    bool installGame(const std::string& path);
    bool runGame();

private:
    EmulatorState state;
    EmulatorConfig config;
    std::string game_path;
    std::string game_title;
    std::string game_id;

    std::unique_ptr<MemoryManager> memory_manager;
    std::unique_ptr<SwitchCPUBackend> cpu;
    std::unique_ptr<ModuleManager> module_manager;
    std::unique_ptr<Renderer> renderer; // <-- Add this line
    std::unique_ptr<SwitchInput> input_system;
    std::unique_ptr<GpuSubsystem> gpu_subsystem; // <-- Add this line
    std::mutex mutex;
};
