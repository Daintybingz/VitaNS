#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <mutex>
#include "vita_memory_layout.h"

// Memory access types for hooks
enum class MemoryAccessType {
    READ,
    WRITE,
    EXECUTE
};

// Memory region types
enum class MemoryRegionType {
    RAM,
    VRAM,
    CDRAM,
    PHYCONT,
    KERNEL,
    SHARED,
    OTHER
};

// Memory protection flags
struct MemoryProtection {
    bool read = false;
    bool write = false;
    bool execute = false;
};

// Memory region structure
struct MemoryRegion {
    MemoryRegionType type;
    uint32_t vita_addr;  // PS Vita virtual address
    uint8_t* host_addr;  // Host memory address
    size_t size;
    MemoryProtection protection;
    std::string name;    // Name for debugging
};

// Memory access hook callback type
using MemoryHookCallback = std::function<void(uint32_t addr, size_t size, MemoryAccessType type, const uint8_t* data)>;

class MemoryManager {
public:
    // Singleton pattern
    static MemoryManager& getInstance();

    // Initialize the memory manager
    bool initialize();
    void finalize();

    // Memory mapping functions
    bool mapVitaMemory();
    MemoryRegion* findRegion(uint32_t vita_addr);
    
    // Address translation
    uint8_t* translateAddress(uint32_t vita_addr);
    uint32_t hostToVita(const void* host_addr);
    
    // Memory access functions with hooks
    template<typename T>
    T read(uint32_t addr) {
        T value = 0;
        read_memory(addr, &value, sizeof(T));
        return value;
    }
    
    template<typename T>
    void write(uint32_t addr, T value) {
        write_memory(addr, &value, sizeof(T));
    }
    
    bool read_memory(uint32_t addr, void* data, size_t size);
    bool write_memory(uint32_t addr, const void* data, size_t size);
    bool execute_memory(uint32_t addr, size_t size);
    
    // Memory hook management
    int addHook(uint32_t addr, size_t size, MemoryAccessType type, MemoryHookCallback callback);
    bool removeHook(int hook_id);
    void clearHooks();
    
    // Memory allocation within PS Vita address space
    uint32_t allocate(size_t size, MemoryRegionType type = MemoryRegionType::RAM, size_t alignment = 0x1000);
    bool free(uint32_t addr);
    
    // Memory protection
    bool protect(uint32_t addr, size_t size, MemoryProtection protection);
    MemoryProtection getProtection(uint32_t addr);
    
    // Debug functions
    void printMemoryMap() const;
    void dumpMemory(uint32_t addr, size_t size, const std::string& filename) const;
    
    // Get total available memory
    size_t getTotalMemory() const;
    size_t getUsedMemory() const;
    size_t getFreeMemory() const;

public:
    MemoryManager();
    ~MemoryManager();

private:
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

    // Memory regions
    std::vector<MemoryRegion> regions;
    std::map<uint32_t, size_t> allocations;  // Maps PS Vita address to allocation size
    
    // Memory hooks
    struct MemoryHook {
        int id;
        uint32_t addr;
        size_t size;
        MemoryAccessType type;
        MemoryHookCallback callback;
    };
    
    std::vector<MemoryHook> hooks;
    int next_hook_id = 1;
    
    // Internal state
    size_t totalMemory;
    size_t usedMemory;
    std::mutex mutex;
    
    // Helper functions
    uint8_t* alignedAlloc(size_t size, size_t alignment);
    void alignedFree(uint8_t* ptr);
    void triggerHooks(uint32_t addr, size_t size, MemoryAccessType type, const uint8_t* data);
    bool checkProtection(uint32_t addr, size_t size, MemoryAccessType type);
    
    // Create a memory region
    bool createRegion(MemoryRegionType type, uint32_t vita_addr, size_t size, 
                     MemoryProtection protection, const std::string& name);
};
