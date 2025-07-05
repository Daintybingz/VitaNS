#pragma once

#include <cstdint>
#include <cstddef>

namespace vita {
namespace memory {

// PS Vita memory layout constants
// Based on official documentation and reverse engineering

// Memory sizes
constexpr size_t RAM_SIZE = 512 * 1024 * 1024;      // 512MB main RAM
constexpr size_t VRAM_SIZE = 128 * 1024 * 1024;     // 128MB video RAM
constexpr size_t CDRAM_SIZE = 4 * 1024 * 1024;      // 4MB CDRAM (Control Display RAM)
constexpr size_t PHYCONT_SIZE = 16 * 1024 * 1024;   // 16MB physically contiguous memory
constexpr size_t TOTAL_MEMORY = RAM_SIZE + VRAM_SIZE + CDRAM_SIZE + PHYCONT_SIZE;

// Memory base addresses (PS Vita virtual address space)
constexpr uint32_t SCE_KERNEL_MAIN_DMACMEM_BASE = 0x00000000;  // DMA controller memory
constexpr uint32_t SCE_KERNEL_MAIN_PSPEMU_BASE = 0x01000000;   // PSP emulator memory
constexpr uint32_t SCE_KERNEL_MAIN_SHARED_BASE = 0x03000000;   // Shared memory
constexpr uint32_t SCE_KERNEL_MAIN_PHYCONT_BASE = 0x10000000;  // Physically contiguous memory
constexpr uint32_t SCE_KERNEL_MAIN_CDRAM_BASE = 0x20000000;    // CDRAM
constexpr uint32_t SCE_KERNEL_MAIN_RAM_BASE = 0x40000000;      // Main RAM
constexpr uint32_t SCE_KERNEL_MAIN_VRAM_BASE = 0x80000000;     // VRAM
constexpr uint32_t SCE_KERNEL_MAIN_KERNEL_BASE = 0xC0000000;   // Kernel memory

// Memory region sizes
constexpr size_t SCE_KERNEL_MAIN_DMACMEM_SIZE = 0x01000000;    // 16MB
constexpr size_t SCE_KERNEL_MAIN_PSPEMU_SIZE = 0x02000000;     // 32MB
constexpr size_t SCE_KERNEL_MAIN_SHARED_SIZE = 0x0D000000;     // 208MB
constexpr size_t SCE_KERNEL_MAIN_PHYCONT_SIZE = PHYCONT_SIZE;  // 16MB
constexpr size_t SCE_KERNEL_MAIN_CDRAM_SIZE = CDRAM_SIZE;      // 4MB
constexpr size_t SCE_KERNEL_MAIN_RAM_SIZE = RAM_SIZE;          // 512MB
constexpr size_t SCE_KERNEL_MAIN_VRAM_SIZE = VRAM_SIZE;        // 128MB
constexpr size_t SCE_KERNEL_MAIN_KERNEL_SIZE = 0x40000000;     // 1GB

// Memory protection flags
constexpr uint32_t SCE_KERNEL_MEMORY_ACCESS_READ = 0x01;
constexpr uint32_t SCE_KERNEL_MEMORY_ACCESS_WRITE = 0x02;
constexpr uint32_t SCE_KERNEL_MEMORY_ACCESS_EXECUTE = 0x04;
constexpr uint32_t SCE_KERNEL_MEMORY_ACCESS_RW = SCE_KERNEL_MEMORY_ACCESS_READ | SCE_KERNEL_MEMORY_ACCESS_WRITE;
constexpr uint32_t SCE_KERNEL_MEMORY_ACCESS_RX = SCE_KERNEL_MEMORY_ACCESS_READ | SCE_KERNEL_MEMORY_ACCESS_EXECUTE;
constexpr uint32_t SCE_KERNEL_MEMORY_ACCESS_RWX = SCE_KERNEL_MEMORY_ACCESS_READ | SCE_KERNEL_MEMORY_ACCESS_WRITE | SCE_KERNEL_MEMORY_ACCESS_EXECUTE;

// Memory types
constexpr uint32_t SCE_KERNEL_MEMORY_TYPE_NORMAL = 0x80;
constexpr uint32_t SCE_KERNEL_MEMORY_TYPE_SHARED = 0x90;
constexpr uint32_t SCE_KERNEL_MEMORY_TYPE_CDRAM = 0xA0;
constexpr uint32_t SCE_KERNEL_MEMORY_TYPE_MAIN = 0xC0;
constexpr uint32_t SCE_KERNEL_MEMORY_TYPE_VRAM = 0xD0;

// Helper functions
inline bool isAddressValid(uint32_t addr) {
    return (addr >= SCE_KERNEL_MAIN_DMACMEM_BASE && addr < (SCE_KERNEL_MAIN_KERNEL_BASE + SCE_KERNEL_MAIN_KERNEL_SIZE));
}

inline bool isAddressInRange(uint32_t addr, uint32_t base, size_t size) {
    return (addr >= base && addr < base + size);
}

inline bool isAddressInMainRam(uint32_t addr) {
    return isAddressInRange(addr, SCE_KERNEL_MAIN_RAM_BASE, SCE_KERNEL_MAIN_RAM_SIZE);
}

inline bool isAddressInVram(uint32_t addr) {
    return isAddressInRange(addr, SCE_KERNEL_MAIN_VRAM_BASE, SCE_KERNEL_MAIN_VRAM_SIZE);
}

inline bool isAddressInCdram(uint32_t addr) {
    return isAddressInRange(addr, SCE_KERNEL_MAIN_CDRAM_BASE, SCE_KERNEL_MAIN_CDRAM_SIZE);
}

inline bool isAddressInPhycont(uint32_t addr) {
    return isAddressInRange(addr, SCE_KERNEL_MAIN_PHYCONT_BASE, SCE_KERNEL_MAIN_PHYCONT_SIZE);
}

} // namespace memory
} // namespace vita
