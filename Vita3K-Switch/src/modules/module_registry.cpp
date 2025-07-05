#include "module_registry.h"
#include "SceLibKernel/SceLibKernel.h"
#include "SceDisplay/SceDisplay.h"
#include "SceCtrl/SceCtrl.h"
#include "SceAudio/SceAudio.h"
#include "SceTouch/SceTouch.h"
#include "../core/emulator/emulator.h"
#include <cstdio>

bool ModuleRegistry::initialize(Emulator& emulator) {
    printf("[ModuleRegistry] Initializing\n");
    
    // Get module manager from emulator
    ModuleManager& manager = emulator.getModuleManager();
    
    // Register all modules
    registerModules(manager);
    
    // Initialize all modules
    return manager.initialize(emulator);
}

void ModuleRegistry::finalize(Emulator& emulator) {
    printf("[ModuleRegistry] Finalizing\n");
    
    // Get module manager from emulator
    ModuleManager& manager = emulator.getModuleManager();
    
    // Finalize all modules
    manager.finalize(emulator);
}

void ModuleRegistry::registerModules(ModuleManager& manager) {
    printf("[ModuleRegistry] Registering modules\n");
    
    // Register SceLibKernel module
    manager.registerModule(SceLibKernel::create());
    
    // Register SceDisplay module
    manager.registerModule(std::make_unique<SceDisplay>());
    
    // Register SceCtrl module
    manager.registerModule(std::make_unique<SceCtrl>());
    
    // Register SceAudio module
    manager.registerModule(std::make_unique<SceAudio>());
    
    // Register SceTouch module
    manager.registerModule(std::make_unique<SceTouch>());
    // etc.
}

int ModuleRegistry::executeSystemCall(Emulator& emulator, NID nid, uint32_t threadId, const std::vector<uint32_t>& args) {
    // Get module manager from emulator
    ModuleManager& manager = emulator.getModuleManager();
    
    // Find function by NID
    ModuleFunction* function = findFunction(manager, nid);
    
    if (!function) {
        printf("[ModuleRegistry] Unknown system call: NID=0x%08X\n", nid);
        return -1;
    }
    
    // Execute function
    printf("[ModuleRegistry] Executing system call: %s (NID=0x%08X)\n", 
        function->name.c_str(), nid);
    
    return function->func(emulator, threadId, args);
}

ModuleFunction* ModuleRegistry::findFunction(ModuleManager& manager, NID nid) {
    return manager.findFunction(nid);
}
