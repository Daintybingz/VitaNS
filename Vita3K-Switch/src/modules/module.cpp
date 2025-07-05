#include "module.h"
#include "../core/emulator/emulator.h"
#include <cstdio>

// Module implementation
Module::Module(const std::string& moduleName)
    : name(moduleName) {
}

Module::~Module() {
}

void Module::registerFunction(const std::string& funcName, NID nid, SysCallFunc func, int argCount) {
    ModuleFunction function;
    function.name = funcName;
    function.nid = nid;
    function.func = func;
    function.argCount = argCount;
    
    functions[nid] = function;
    
    printf("[Module] Registered function %s (NID: 0x%08X) in module %s\n", 
        funcName.c_str(), nid, name.c_str());
}

ModuleFunction* Module::findFunction(NID nid) {
    auto it = functions.find(nid);
    if (it != functions.end()) {
        return &it->second;
    }
    return nullptr;
}

const std::string& Module::getName() const {
    return name;
}

const std::map<NID, ModuleFunction>& Module::getFunctions() const {
    return functions;
}

bool Module::initialize(Emulator& emulator) {
    printf("[Module] Initializing module %s\n", name.c_str());
    return true;
}

void Module::finalize(Emulator& emulator) {
    printf("[Module] Finalizing module %s\n", name.c_str());
}

// Module manager implementation
ModuleManager::ModuleManager() {
}

ModuleManager::~ModuleManager() {
}

void ModuleManager::registerModule(std::shared_ptr<Module> module) {
    const std::string& name = module->getName();
    modules[name] = module;
    
    // Register all functions in the NID map
    for (auto& pair : module->getFunctions()) {
        nidToFunction[pair.first] = pair.second;
    }
    
    printf("[ModuleManager] Registered module %s\n", name.c_str());
}

std::shared_ptr<Module> ModuleManager::findModule(const std::string& name) {
    auto it = modules.find(name);
    if (it != modules.end()) {
        return it->second;
    }
    return nullptr;
}

ModuleFunction* ModuleManager::findFunction(NID nid) {
    auto it = nidToFunction.find(nid);
    if (it != nidToFunction.end()) {
        return &it->second;
    }
    return nullptr;
}

bool ModuleManager::initialize(Emulator& emulator) {
    printf("[ModuleManager] Initializing modules\n");
    
    bool success = true;
    for (auto& pair : modules) {
        if (!pair.second->initialize(emulator)) {
            printf("[ModuleManager] Failed to initialize module %s\n", pair.first.c_str());
            success = false;
        }
    }
    
    return success;
}

void ModuleManager::finalize(Emulator& emulator) {
    printf("[ModuleManager] Finalizing modules\n");
    
    for (auto& pair : modules) {
        pair.second->finalize(emulator);
    }
    
    modules.clear();
    nidToFunction.clear();
}
