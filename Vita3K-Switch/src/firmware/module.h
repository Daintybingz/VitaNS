#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

namespace firmware {

// Module information structure
struct ModuleInfo {
    std::string name;
    uint32_t version;
    uint32_t attributes;
    fs::path path;
    bool is_loaded;
    bool is_started;
};

// Module type enumeration
enum class ModuleType {
    USER,       // User-mode module
    KERNEL,     // Kernel-mode module
    SYSTEM      // System module
};

// Forward declarations
class Module;

// Module manager class
class ModuleManager {
public:
    ModuleManager();
    ~ModuleManager();

    // Initialize the module manager
    bool initialize(const fs::path &firmware_path);

    // Load a module by name
    std::shared_ptr<Module> load_module(const std::string &name, ModuleType type = ModuleType::USER);

    // Load a module by path
    std::shared_ptr<Module> load_module_by_path(const fs::path &path, ModuleType type = ModuleType::USER);

    // Unload a module
    bool unload_module(const std::string &name);

    // Start a module
    bool start_module(const std::string &name);

    // Stop a module
    bool stop_module(const std::string &name);

    // Get a loaded module by name
    std::shared_ptr<Module> get_module(const std::string &name);

    // Get all loaded modules
    std::vector<std::shared_ptr<Module>> get_all_modules();

    // Get all module information
    std::vector<ModuleInfo> get_all_module_info();

    // Scan for available modules
    bool scan_modules(const fs::path &path);

    // Add stubs for missing methods
    void unload_all_modules() {}
    void set_system_module_path(const fs::path&) {}
    void set_user_module_path(const fs::path&) {}

private:
    // Map of module name to module object
    std::map<std::string, std::shared_ptr<Module>> loaded_modules;

    // Map of module name to module information
    std::map<std::string, ModuleInfo> available_modules;

    // Firmware path
    fs::path firmware_path;
};

// Module base class
class Module {
public:
    Module(const std::string &name, ModuleType type);
    virtual ~Module();

    // Load the module
    virtual bool load(const fs::path &path);

    // Unload the module
    virtual bool unload();

    // Start the module
    virtual bool start();

    // Stop the module
    virtual bool stop();

    // Get module information
    virtual ModuleInfo get_info() const;

    // Get module name
    std::string get_name() const;

    // Get module type
    ModuleType get_type() const;

    // Check if module is loaded
    bool is_loaded() const;

    // Check if module is started
    bool is_started() const;

protected:
    // Module name
    std::string name;

    // Module type
    ModuleType type;

    // Module path
    fs::path path;

    // Module version
    uint32_t version;

    // Module attributes
    uint32_t attributes;

    // Loaded flag
    bool loaded;

    // Started flag
    bool started;
};

// Create a module instance based on type
std::shared_ptr<Module> create_module(const std::string &name, ModuleType type);

} // namespace firmware
