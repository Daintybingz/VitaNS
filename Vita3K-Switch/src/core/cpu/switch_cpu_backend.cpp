#include "switch_cpu_backend.h"
#include "../emulator/emulator.h"
#include "../../modules/SceCtrl/SceCtrl.h"
#include "../../modules/SceAudio/SceAudio.h"
#include <cstdio>
#include <vector>

SwitchCPUBackend::SwitchCPUBackend()
    : running(false), log_code(false), log_mem(false), emulator(nullptr), threadId(0), optimization_enabled(false) {
    // Initialize context to zero
    std::fill_n(context.regs, 16, 0);
    context.cpsr = 0;
    std::fill_n(context.fregs, 32, 0.0f);
    context.fpscr = 0;
    
    // Create ARM translator (disabled by default)
    translator = createARMTranslator(false); // Direct translator without JIT for now
}

void SwitchCPUBackend::setEmulator(Emulator* emu) {
    emulator = emu;
    threadId = 0x1000; // Default main thread ID
}

int SwitchCPUBackend::executeSystemCall(uint32_t nid, const std::vector<uint32_t>& args) {
    if (!emulator) {
        printf("[SwitchCPUBackend] Cannot execute system call: no emulator reference\n");
        return -1;
    }
    
    // return emulator->executeSystemCall(nid, threadId, args);
    return 0;
}

SwitchCPUBackend::~SwitchCPUBackend() {
    stop();
    translator.reset();
}

int SwitchCPUBackend::run() {
    running = true;
    
    if (optimization_enabled && translator) {
        return runOptimized();
    } else {
        return runInterpreted();
    }
}

int SwitchCPUBackend::runOptimized() {
    if (!translator) {
        printf("[SwitchCPUBackend] Error: No translator available\n");
        return -1;
    }
    
    while (running) {
        // Get current PC
        uint32_t pc = get_pc();
        
        // Check if we should stop
        if (hit_breakpoint()) {
            printf("[SwitchCPUBackend] Breakpoint hit at 0x%08X\n", pc);
            break;
        }
        
        // Translate and execute the block at the current PC
        bool is_thumb = is_thumb_mode();
        TranslationBlock* block = translator->translateBlock(pc, is_thumb);
        if (!block) {
            printf("[SwitchCPUBackend] Error: Failed to translate block at 0x%08X\n", pc);
            break;
        }
        
        // Execute the block
        int result = translator->executeBlock(block, context);
        if (result != 0) {
            printf("[SwitchCPUBackend] Error: Failed to execute block at 0x%08X\n", pc);
            break;
        }
        
        // For demonstration purposes, just advance PC by a fixed amount
        // In a real implementation, the translator would update the PC
        set_pc(pc + 4);
        
        // Example: Call system functions occasionally
        if ((get_pc() & 0xFF) == 0) {
            // Example: Call sceCtrlPeekBufferPositive
            std::vector<uint32_t> args = {0x1000, 1, 0};
            executeSystemCall(0x9B96A1AA, args);
        }
    }
    
    running = false;
    return 0;
}

int SwitchCPUBackend::runInterpreted() {
    // Original stub implementation
    while (running) {
        // Execute system calls for testing
        if (emulator) {
            // Example: Call sceCtrlPeekBufferPositive
            std::vector<uint32_t> args = {0x1000, 1, 0};
            executeSystemCall(0x9B96A1AA, args);
            
            // Example: Call sceAudioOutOutput
            args = {0, 0};
            executeSystemCall(0x02DB3F5F, args);
            
            // Advance PC for demonstration
            set_pc(get_pc() + 4);
        }
        
        // For demonstration, just run a few iterations
        break;
    }
    
    running = false;
    return 0;
}

uint32_t SwitchCPUBackend::get_reg(uint8_t idx) {
    if (idx < 16) return context.regs[idx];
    return 0;
}

void SwitchCPUBackend::set_reg(uint8_t idx, uint32_t val) {
    if (idx < 16) context.regs[idx] = val;
}

uint32_t SwitchCPUBackend::get_sp() { return context.regs[13]; }
void SwitchCPUBackend::set_sp(uint32_t val) { context.regs[13] = val; }
uint32_t SwitchCPUBackend::get_pc() { return context.regs[15]; }
void SwitchCPUBackend::set_pc(uint32_t val) { context.regs[15] = val; }
uint32_t SwitchCPUBackend::get_lr() { return context.regs[14]; }
void SwitchCPUBackend::set_lr(uint32_t val) { context.regs[14] = val; }
uint32_t SwitchCPUBackend::get_cpsr() { return context.cpsr; }
void SwitchCPUBackend::set_cpsr(uint32_t val) { context.cpsr = val; }

float SwitchCPUBackend::get_float_reg(uint8_t idx) {
    if (idx < 32) return context.fregs[idx];
    return 0.0f;
}
void SwitchCPUBackend::set_float_reg(uint8_t idx, float val) {
    if (idx < 32) context.fregs[idx] = val;
}
uint32_t SwitchCPUBackend::get_fpscr() { return context.fpscr; }
void SwitchCPUBackend::set_fpscr(uint32_t val) { context.fpscr = val; }

CPUContext SwitchCPUBackend::save_context() {
    return context;
}
void SwitchCPUBackend::load_context(const CPUContext &ctx) {
    context = ctx;
}

void SwitchCPUBackend::invalidate_jit_cache(uintptr_t start, size_t length) {
    if (translator && optimization_enabled) {
        translator->invalidateRange(start, start + length);
    }
}

bool SwitchCPUBackend::is_thumb_mode() {
    // TODO: Return true if CPSR thumb bit is set
    return (context.cpsr & (1 << 5)) != 0;
}

int SwitchCPUBackend::step() {
    // Execute a single instruction
    if (optimization_enabled && translator) {
        // In optimized mode, we execute a single instruction through the translator
        uint32_t pc = get_pc();
        bool is_thumb = is_thumb_mode();
        
        // For demonstration, just advance PC
        set_pc(pc + (is_thumb ? 2 : 4));
        
        printf("[SwitchCPUBackend] step() called (optimized)\n");
    } else {
        // In interpreted mode, we execute a single instruction directly
        uint32_t pc = get_pc();
        bool is_thumb = is_thumb_mode();
        
        // For demonstration, just advance PC
        set_pc(pc + (is_thumb ? 2 : 4));
        
        printf("[SwitchCPUBackend] step() called (interpreted)\n");
    }
    
    return 0;
}

bool SwitchCPUBackend::hit_breakpoint() {
    // TODO: Implement breakpoint check
    return false;
}
void SwitchCPUBackend::trigger_breakpoint() {
    // TODO: Implement breakpoint trigger
}
void SwitchCPUBackend::set_log_code(bool log) { log_code = log; }
void SwitchCPUBackend::set_log_mem(bool log) { log_mem = log; }
bool SwitchCPUBackend::get_log_code() { return log_code; }
bool SwitchCPUBackend::get_log_mem() { return log_mem; }

void SwitchCPUBackend::stop() {
    // Stub: nothing to do
}
