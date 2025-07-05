#ifndef DYNARMIC_MEMORY_INTERFACE_H
#define DYNARMIC_MEMORY_INTERFACE_H

#pragma once

#include "core/cpu/arm_translator.h"
#include <cstdint>

class DynarmicMemoryInterface {
public:
    DynarmicMemoryInterface(ICPUBackend* cpu_backend);
    ~DynarmicMemoryInterface();

    // Memory read callbacks
    uint8_t ReadMemory8(uint32_t vaddr);
    uint16_t ReadMemory16(uint32_t vaddr);
    uint32_t ReadMemory32(uint32_t vaddr);
    uint64_t ReadMemory64(uint32_t vaddr);

    // Memory write callbacks
    void WriteMemory8(uint32_t vaddr, uint8_t value);
    void WriteMemory16(uint32_t vaddr, uint16_t value);
    void WriteMemory32(uint32_t vaddr, uint32_t value);
    void WriteMemory64(uint32_t vaddr, uint64_t value);

    // Exclusive memory access (for ARM exclusive instructions)
    bool ExclusiveRead8(uint32_t vaddr, uint8_t* value);
    bool ExclusiveRead16(uint32_t vaddr, uint16_t* value);
    bool ExclusiveRead32(uint32_t vaddr, uint32_t* value);
    bool ExclusiveRead64(uint32_t vaddr, uint64_t* value);
    
    bool ExclusiveWrite8(uint32_t vaddr, uint8_t value);
    bool ExclusiveWrite16(uint32_t vaddr, uint16_t value);
    bool ExclusiveWrite32(uint32_t vaddr, uint32_t value);
    bool ExclusiveWrite64(uint32_t vaddr, uint64_t value);

    // Memory permissions
    bool IsReadable(uint32_t vaddr, uint32_t access_size = 1);
    bool IsWritable(uint32_t vaddr, uint32_t access_size = 1);
    bool IsExecutable(uint32_t vaddr, uint32_t access_size = 1);

private:
    ICPUBackend* cpu_backend;
    uint32_t exclusive_monitor_addr;
    bool has_exclusive_monitor;
};

#endif // DYNARMIC_MEMORY_INTERFACE_H 