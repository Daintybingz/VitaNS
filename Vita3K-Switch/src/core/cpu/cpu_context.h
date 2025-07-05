#pragma once

#include <cstdint>
#include <array>

// CPU context structure for ARM emulation
struct CPUContext {
    // General purpose registers (r0-r15, where r15 is PC)
    std::array<uint32_t, 16> regs;
    
    // Current Program Status Register (CPSR)
    uint32_t cpsr;
    
    // Floating point registers (optional, for VFP support)
    std::array<float, 32> vfp_regs;
    
    // Floating point status register
    uint32_t fpscr;
    
    // Constructor to initialize the context
    CPUContext() {
        // Initialize all registers to 0
        regs.fill(0);
        cpsr = 0;
        vfp_regs.fill(0.0f);
        fpscr = 0;
    }
    
    // Helper methods
    
    // Check if in Thumb mode
    bool isThumbMode() const {
        return (cpsr & (1 << 5)) != 0;
    }
    
    // Set Thumb mode
    void setThumbMode(bool thumb) {
        if (thumb) {
            cpsr |= (1 << 5);
        } else {
            cpsr &= ~(1 << 5);
        }
    }
    
    // Get PC
    uint32_t getPC() const {
        return regs[15];
    }
    
    // Set PC
    void setPC(uint32_t pc) {
        regs[15] = pc;
    }
    
    // Get Link Register (LR)
    uint32_t getLR() const {
        return regs[14];
    }
    
    // Set Link Register (LR)
    void setLR(uint32_t lr) {
        regs[14] = lr;
    }
    
    // Get Stack Pointer (SP)
    uint32_t getSP() const {
        return regs[13];
    }
    
    // Set Stack Pointer (SP)
    void setSP(uint32_t sp) {
        regs[13] = sp;
    }
};
