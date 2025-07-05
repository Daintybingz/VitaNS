#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <cstdint>

// Forward declarations
class Emulator;

// System call function type
using SysCallFunc = std::function<int(Emulator&, uint32_t, const std::vector<uint32_t>&)>;

// NID (Network ID) type - used to identify functions in PS Vita modules
using NID = uint32_t;

// Module function definition
struct ModuleFunction {
    std::string name;
    NID nid;
    SysCallFunc func;
    int argCount;
};

// Module class - represents a PS Vita system library
class Module {
public:
    Module(const std::string& name);
    virtual ~Module();

    // Register a function in this module
    void registerFunction(const std::string& name, NID nid, SysCallFunc func, int argCount);
    
    // Find a function by NID
    ModuleFunction* findFunction(NID nid);
    
    // Get module name
    const std::string& getName() const;
    
    // Get all functions
    const std::map<NID, ModuleFunction>& getFunctions() const;
    
    // Initialize the module
    virtual bool initialize(Emulator& emulator);
    
    // Finalize the module
    virtual void finalize(Emulator& emulator);

    // Register module functions
    virtual void registerFunctions() = 0;

private:
    std::string name;
    std::map<NID, ModuleFunction> functions;
};

// Module manager - handles all modules
class ModuleManager {
public:
    ModuleManager();
    ~ModuleManager();
    
    // Register a module
    void registerModule(std::shared_ptr<Module> module);
    
    // Find a module by name
    std::shared_ptr<Module> findModule(const std::string& name);
    
    // Find a function by NID across all modules
    ModuleFunction* findFunction(NID nid);
    
    // Initialize all modules
    bool initialize(Emulator& emulator);
    
    // Finalize all modules
    void finalize(Emulator& emulator);

private:
    std::map<std::string, std::shared_ptr<Module>> modules;
    std::map<NID, ModuleFunction> nidToFunction;
};
