#pragma once

#include "module.h"
#include <memory>
#include <vector>

// Forward declarations
class Emulator;

// Module registry - manages all PS Vita system modules
class ModuleRegistry {
public:
    // Initialize the registry
    static bool initialize(Emulator& emulator);
    
    // Finalize the registry
    static void finalize(Emulator& emulator);
    
    // Register all modules
    static void registerModules(ModuleManager& manager);
    
    // Execute a system call
    static int executeSystemCall(Emulator& emulator, NID nid, uint32_t threadId, const std::vector<uint32_t>& args);
    
    // Find a function by NID
    static ModuleFunction* findFunction(ModuleManager& manager, NID nid);
};
