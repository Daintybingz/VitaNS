#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>
#include "arm_translator.h"

// Placeholder CPU context structure (expand as needed)
struct CPUContext {
    uint32_t regs[16];   // General-purpose registers
    uint32_t cpsr;       // Current program status register
    float    fregs[32];  // Floating-point registers
    uint32_t fpscr;      // Floating-point status/control
};

// Abstract CPU interface for emulator core compatibility
class ICPUBackend {
public:
    virtual ~ICPUBackend() = default;
    virtual int run() = 0;
    virtual void stop() = 0;
    virtual uint32_t get_reg(uint8_t idx) = 0;
    virtual void set_reg(uint8_t idx, uint32_t val) = 0;
    virtual uint32_t get_sp() = 0;
    virtual void set_sp(uint32_t val) = 0;
    virtual uint32_t get_pc() = 0;
    virtual void set_pc(uint32_t val) = 0;
    virtual uint32_t get_lr() = 0;
    virtual void set_lr(uint32_t val) = 0;
    virtual uint32_t get_cpsr() = 0;
    virtual void set_cpsr(uint32_t val) = 0;
    virtual float get_float_reg(uint8_t idx) = 0;
    virtual void set_float_reg(uint8_t idx, float val) = 0;
    virtual uint32_t get_fpscr() = 0;
    virtual void set_fpscr(uint32_t val) = 0;
    virtual CPUContext save_context() = 0;
    virtual void load_context(const CPUContext &ctx) = 0;
    virtual void invalidate_jit_cache(uintptr_t start, size_t length) = 0;
    virtual bool is_thumb_mode() = 0;
    virtual int step() = 0;
    virtual bool hit_breakpoint() = 0;
    virtual void trigger_breakpoint() = 0;
    virtual void set_log_code(bool log) = 0;
    virtual void set_log_mem(bool log) = 0;
    virtual bool get_log_code() = 0;
    virtual bool get_log_mem() = 0;
};

// Forward declarations
class Emulator;

// Switch-specific CPU backend (stub)
class SwitchCPUBackend : public ICPUBackend {
public:
    SwitchCPUBackend();
    ~SwitchCPUBackend();
    
    // Set emulator reference
    void setEmulator(Emulator* emulator);
    
    // System call handling
    int executeSystemCall(uint32_t nid, const std::vector<uint32_t>& args);

    // Core emulation loop
    int run() override;
    void stop() override;

    // Register access
    uint32_t get_reg(uint8_t idx) override;
    void set_reg(uint8_t idx, uint32_t val) override;
    uint32_t get_sp() override;
    void set_sp(uint32_t val) override;
    uint32_t get_pc() override;
    void set_pc(uint32_t val) override;
    uint32_t get_lr() override;
    void set_lr(uint32_t val) override;
    uint32_t get_cpsr() override;
    void set_cpsr(uint32_t val) override;
    float get_float_reg(uint8_t idx) override;
    void set_float_reg(uint8_t idx, float val) override;
    uint32_t get_fpscr() override;
    void set_fpscr(uint32_t val) override;

    // Context save/load
    CPUContext save_context() override;
    void load_context(const CPUContext &ctx) override;

    // JIT cache control
    void invalidate_jit_cache(uintptr_t start, size_t length) override;

    // Thumb mode
    bool is_thumb_mode() override;
    int step() override;

    // Debug
    bool hit_breakpoint() override;
    void trigger_breakpoint() override;
    void set_log_code(bool log) override;
    void set_log_mem(bool log) override;
    bool get_log_code() override;
    bool get_log_mem() override;
    
    // ARM translation optimization
    bool enableOptimization(bool enable);
    bool isOptimizationEnabled() const;
    void printTranslationStats() const;

private:
    CPUContext context;
    bool running;
    bool log_code;
    bool log_mem;
    Emulator* emulator;
    uint32_t threadId;
    
    // ARM translation
    std::unique_ptr<ARMTranslator> translator;
    bool optimization_enabled;
    
    // Helper methods
    int runOptimized();
    int runInterpreted();
};
