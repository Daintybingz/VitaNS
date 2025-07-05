#include "module.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace firmware {

// Module implementation
Module::Module(const std::string &name, ModuleType type)
    : name(name), type(type), version(0), attributes(0), loaded(false), started(false) {
}

Module::~Module() {
    if (started) {
        stop();
    }
    if (loaded) {
        unload();
    }
}

bool Module::load(const fs::path &path) {
    if (loaded) {
        std::cerr << "Module " << name << " is already loaded" << std::endl;
        return false;
    }

    this->path = path;

    // In a real implementation, we would load the module file here
    // For now, just set the loaded flag to true
    std::cout << "Loading module " << name << " from " << path << std::endl;
    loaded = true;

    return true;
}

bool Module::unload() {
    if (!loaded) {
        std::cerr << "Module " << name << " is not loaded" << std::endl;
        return false;
    }

    if (started) {
        std::cerr << "Module " << name << " is still running, stop it first" << std::endl;
        return false;
    }

    // In a real implementation, we would unload the module here
    // For now, just set the loaded flag to false
    std::cout << "Unloading module " << name << std::endl;
    loaded = false;

    return true;
}

bool Module::start() {
    if (!loaded) {
        std::cerr << "Module " << name << " is not loaded" << std::endl;
        return false;
    }

    if (started) {
        std::cerr << "Module " << name << " is already started" << std::endl;
        return false;
    }

    // In a real implementation, we would start the module here
    // For now, just set the started flag to true
    std::cout << "Starting module " << name << std::endl;
    started = true;

    return true;
}

bool Module::stop() {
    if (!loaded) {
        std::cerr << "Module " << name << " is not loaded" << std::endl;
        return false;
    }

    if (!started) {
        std::cerr << "Module " << name << " is not started" << std::endl;
        return false;
    }

    // In a real implementation, we would stop the module here
    // For now, just set the started flag to false
    std::cout << "Stopping module " << name << std::endl;
    started = false;

    return true;
}

ModuleInfo Module::get_info() const {
    ModuleInfo info;
    info.name = name;
    info.version = version;
    info.attributes = attributes;
    info.path = path;
    info.is_loaded = loaded;
    info.is_started = started;
    return info;
}

std::string Module::get_name() const {
    return name;
}

ModuleType Module::get_type() const {
    return type;
}

bool Module::is_loaded() const {
    return loaded;
}

bool Module::is_started() const {
    return started;
}

// User module implementation
class UserModule : public Module {
public:
    UserModule(const std::string &name) : Module(name, ModuleType::USER) {}

    bool load(const fs::path &path) override {
        if (Module::load(path)) {
            // Additional user module specific loading
            std::cout << "Loading user module " << name << std::endl;
            return true;
        }
        return false;
    }

    bool start() override {
        if (Module::start()) {
            // Additional user module specific starting
            std::cout << "Starting user module " << name << std::endl;
            return true;
        }
        return false;
    }
};

// Kernel module implementation
class KernelModule : public Module {
public:
    KernelModule(const std::string &name) : Module(name, ModuleType::KERNEL) {}

    bool load(const fs::path &path) override {
        if (Module::load(path)) {
            // Additional kernel module specific loading
            std::cout << "Loading kernel module " << name << std::endl;
            return true;
        }
        return false;
    }

    bool start() override {
        if (Module::start()) {
            // Additional kernel module specific starting
            std::cout << "Starting kernel module " << name << std::endl;
            return true;
        }
        return false;
    }
};

// System module implementation
class SystemModule : public Module {
public:
    SystemModule(const std::string &name) : Module(name, ModuleType::SYSTEM) {}

    bool load(const fs::path &path) override {
        if (Module::load(path)) {
            // Additional system module specific loading
            std::cout << "Loading system module " << name << std::endl;
            return true;
        }
        return false;
    }

    bool start() override {
        if (Module::start()) {
            // Additional system module specific starting
            std::cout << "Starting system module " << name << std::endl;
            return true;
        }
        return false;
    }
};

// Create a module instance based on type
std::shared_ptr<Module> create_module(const std::string &name, ModuleType type) {
    switch (type) {
        case ModuleType::USER:
            return std::make_shared<UserModule>(name);
        case ModuleType::KERNEL:
            return std::make_shared<KernelModule>(name);
        case ModuleType::SYSTEM:
            return std::make_shared<SystemModule>(name);
        default:
            return nullptr;
    }
}

// ModuleManager implementation
ModuleManager::ModuleManager() {
}

ModuleManager::~ModuleManager() {
    // Unload all modules
    for (auto &pair : loaded_modules) {
        auto module = pair.second;
        if (module->is_started()) {
            module->stop();
        }
        if (module->is_loaded()) {
            module->unload();
        }
    }
    loaded_modules.clear();
}

bool ModuleManager::initialize(const fs::path &firmware_path) {
    this->firmware_path = firmware_path;

    // Scan for available modules
    if (!scan_modules(firmware_path / "vs0" / "modules")) {
        std::cerr << "Failed to scan modules" << std::endl;
        return false;
    }

    std::cout << "Module manager initialized with " << available_modules.size() << " available modules" << std::endl;
    return true;
}

std::shared_ptr<Module> ModuleManager::load_module(const std::string &name, ModuleType type) {
    // Check if module is already loaded
    auto it = loaded_modules.find(name);
    if (it != loaded_modules.end()) {
        std::cout << "Module " << name << " is already loaded" << std::endl;
        return it->second;
    }

    // Check if module is available
    auto info_it = available_modules.find(name);
    if (info_it == available_modules.end()) {
        std::cerr << "Module " << name << " is not available" << std::endl;
        return nullptr;
    }

    // Create module instance
    auto module = create_module(name, type);
    if (!module) {
        std::cerr << "Failed to create module " << name << std::endl;
        return nullptr;
    }

    // Load the module
    if (!module->load(info_it->second.path)) {
        std::cerr << "Failed to load module " << name << std::endl;
        return nullptr;
    }

    // Add to loaded modules
    loaded_modules[name] = module;

    return module;
}

std::shared_ptr<Module> ModuleManager::load_module_by_path(const fs::path &path, ModuleType type) {
    if (!fs::exists(path)) {
        spdlog::error("Module file does not exist: {}", path.string());
        return nullptr;
    }

    // Extract module name from path
    std::string name = path.filename().string();
    
    // Remove extension if present
    size_t dot_pos = name.find_last_of('.');
    if (dot_pos != std::string::npos) {
        name = name.substr(0, dot_pos);
    }

    // Check if module is already loaded
    auto it = loaded_modules.find(name);
    if (it != loaded_modules.end()) {
        std::cout << "Module " << name << " is already loaded" << std::endl;
        return it->second;
    }

    // Create module instance
    auto module = create_module(name, type);
    if (!module) {
        std::cerr << "Failed to create module " << name << std::endl;
        return nullptr;
    }

    // Load the module
    if (!module->load(path)) {
        std::cerr << "Failed to load module " << name << std::endl;
        return nullptr;
    }

    // Add to loaded modules
    loaded_modules[name] = module;

    return module;
}

bool ModuleManager::unload_module(const std::string &name) {
    // Check if module is loaded
    auto it = loaded_modules.find(name);
    if (it == loaded_modules.end()) {
        std::cerr << "Module " << name << " is not loaded" << std::endl;
        return false;
    }

    // Get module
    auto module = it->second;

    // Stop module if started
    if (module->is_started()) {
        if (!module->stop()) {
            std::cerr << "Failed to stop module " << name << std::endl;
            return false;
        }
    }

    // Unload module
    if (!module->unload()) {
        std::cerr << "Failed to unload module " << name << std::endl;
        return false;
    }

    // Remove from loaded modules
    loaded_modules.erase(it);

    return true;
}

bool ModuleManager::start_module(const std::string &name) {
    // Check if module is loaded
    auto it = loaded_modules.find(name);
    if (it == loaded_modules.end()) {
        std::cerr << "Module " << name << " is not loaded" << std::endl;
        return false;
    }

    // Get module
    auto module = it->second;

    // Start module
    if (!module->start()) {
        std::cerr << "Failed to start module " << name << std::endl;
        return false;
    }

    return true;
}

bool ModuleManager::stop_module(const std::string &name) {
    // Check if module is loaded
    auto it = loaded_modules.find(name);
    if (it == loaded_modules.end()) {
        std::cerr << "Module " << name << " is not loaded" << std::endl;
        return false;
    }

    // Get module
    auto module = it->second;

    // Stop module
    if (!module->stop()) {
        std::cerr << "Failed to stop module " << name << std::endl;
        return false;
    }

    return true;
}

std::shared_ptr<Module> ModuleManager::get_module(const std::string &name) {
    // Check if module is loaded
    auto it = loaded_modules.find(name);
    if (it == loaded_modules.end()) {
        return nullptr;
    }

    return it->second;
}

std::vector<std::shared_ptr<Module>> ModuleManager::get_all_modules() {
    std::vector<std::shared_ptr<Module>> modules;
    for (auto &pair : loaded_modules) {
        modules.push_back(pair.second);
    }
    return modules;
}

std::vector<ModuleInfo> ModuleManager::get_all_module_info() {
    std::vector<ModuleInfo> info;
    for (auto &pair : available_modules) {
        info.push_back(pair.second);
    }
    return info;
}

bool ModuleManager::scan_modules(const fs::path &path) {
    if (!fs::exists(path)) {
        std::cerr << "Module path " << path << " does not exist" << std::endl;
        return false;
    }

    // Clear available modules
    available_modules.clear();

    // Scan for modules
    try {
        for (const auto &entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                const auto &file_path = entry.path();
                const auto &extension = file_path.extension().string();
                
                // Check if file is a module (SELF or SUPRX)
                if (extension == ".self" || extension == ".suprx") {
                    // Extract module name
                    std::string name = file_path.filename().string();
                    
                    // Remove extension
                    size_t dot_pos = name.find_last_of('.');
                    if (dot_pos != std::string::npos) {
                        name = name.substr(0, dot_pos);
                    }
                    
                    // Create module info
                    ModuleInfo info;
                    info.name = name;
                    info.path = file_path;
                    info.is_loaded = false;
                    info.is_started = false;
                    
                    // Add to available modules
                    available_modules[name] = info;
                    
                    std::cout << "Found module: " << name << " at " << file_path << std::endl;
                }
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Error scanning modules: " << e.what() << std::endl;
        return false;
    }

    return true;
}

} // namespace firmware
