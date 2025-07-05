#pragma once

#include "cpu_context.h"
#include <cstdint>
#include <string>

// Interface for CPU backend implementations
class ICPUBackend {
public:
    virtual ~ICPUBackend() = default;
    
    // Initialize the CPU backend
    virtual bool initialize() = 0;
    
    // Run the CPU
    virtual int run() = 0;
    
    // Stop the CPU
    virtual void stop() = 0;
    
    // Reset the CPU
    virtual void reset() = 0;
    
    // Step one instruction
    virtual void step() = 0;
    
    // Get CPU context
    virtual CPUContext& getContext() = 0;
    
    // Set CPU context
    virtual void setContext(const CPUContext& context) = 0;
    
    // Memory access methods
    
    // Translate a PS Vita address to a host address
    virtual void* translateAddress(uint32_t addr) = 0;
    
    // Read memory
    virtual bool readMemory(uint32_t addr, void* buffer, size_t size) = 0;
    
    // Write memory
    virtual bool writeMemory(uint32_t addr, const void* buffer, size_t size) = 0;
    
    // Check if memory is writable
    virtual bool isWritable(uint32_t addr) = 0;
    
    // Optimization control
    
    // Enable optimization
    virtual void enableOptimization() = 0;
    
    // Disable optimization
    virtual void disableOptimization() = 0;
    
    // Check if optimization is enabled
    virtual bool isOptimizationEnabled() const = 0;
    
    // Get optimization statistics
    virtual std::string getOptimizationStats() const = 0;
    
    // Invalidate translated code at address
    virtual void invalidateCode(uint32_t addr) = 0;
    
    // Invalidate translated code in range
    virtual void invalidateCodeRange(uint32_t start_addr, uint32_t end_addr) = 0;
};

// Switch CPU backend implementation
class SwitchCPUBackend : public ICPUBackend {
public:
    SwitchCPUBackend();
    ~SwitchCPUBackend();
    
    bool initialize() override;
    int run() override;
    void stop() override;
    void reset() override;
    void step() override;
    CPUContext& getContext() override;
    void setContext(const CPUContext& context) override;
    void* translateAddress(uint32_t addr) override;
    bool readMemory(uint32_t addr, void* buffer, size_t size) override;
    bool writeMemory(uint32_t addr, const void* buffer, size_t size) override;
    bool isWritable(uint32_t addr) override;
    void enableOptimization() override;
    void disableOptimization() override;
    bool isOptimizationEnabled() const override;
    std::string getOptimizationStats() const override;
    void invalidateCode(uint32_t addr) override;
    void invalidateCodeRange(uint32_t start_addr, uint32_t end_addr) override;
    
    // Switch-specific methods
    int runOptimized();
    int runInterpreted();
    
private:
    // CPU context
    CPUContext context;
    
    // Optimization flag
    bool optimization_enabled;
    
    // Running flag
    bool running;
    
    // ARM translator for optimized execution
    class ARMTranslator* translator;
};
