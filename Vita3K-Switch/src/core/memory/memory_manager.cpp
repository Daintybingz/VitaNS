#include "memory_manager.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <algorithm>

// Singleton instance
MemoryManager& MemoryManager::getInstance() {
    static MemoryManager instance;
    return instance;
}

MemoryManager::MemoryManager() 
    : totalMemory(vita::memory::TOTAL_MEMORY), usedMemory(0), next_hook_id(1) {}

MemoryManager::~MemoryManager() { 
    finalize(); 
}

bool MemoryManager::initialize() {
    printf("[MemoryManager] Initializing memory system...\n");
    
    // Map PS Vita memory regions
    if (!mapVitaMemory()) {
        printf("[MemoryManager] Failed to map PS Vita memory!\n");
        return false;
    }
    
    printf("[MemoryManager] Memory system initialized successfully\n");
    printMemoryMap();
    return true;
}

void MemoryManager::finalize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Free all memory regions
    for (auto& region : regions) {
        if (region.host_addr) {
            printf("[MemoryManager] Freeing region: %s\n", region.name.c_str());
            alignedFree(region.host_addr);
            region.host_addr = nullptr;
        }
    }
    
    // Clear all data structures
    regions.clear();
    allocations.clear();
    hooks.clear();
    usedMemory = 0;
    
    printf("[MemoryManager] Memory system finalized\n");
}

bool MemoryManager::mapVitaMemory() {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Create memory protection with full access
    MemoryProtection rwx = {true, true, true};
    MemoryProtection rx = {true, false, true};
    MemoryProtection rw = {true, true, false};
    MemoryProtection ro = {true, false, false};
    
    // Map main RAM
    if (!createRegion(MemoryRegionType::RAM, 
                      vita::memory::SCE_KERNEL_MAIN_RAM_BASE, 
                      vita::memory::RAM_SIZE, 
                      rwx, "Main RAM")) {
        printf("[MemoryManager] Failed to map main RAM!\n");
        return false;
    }
    
    // Map VRAM
    if (!createRegion(MemoryRegionType::VRAM, 
                      vita::memory::SCE_KERNEL_MAIN_VRAM_BASE, 
                      vita::memory::VRAM_SIZE, 
                      rw, "VRAM")) {
        printf("[MemoryManager] Failed to map VRAM!\n");
        return false;
    }
    
    // Map CDRAM
    if (!createRegion(MemoryRegionType::CDRAM, 
                      vita::memory::SCE_KERNEL_MAIN_CDRAM_BASE, 
                      vita::memory::CDRAM_SIZE, 
                      rw, "CDRAM")) {
        printf("[MemoryManager] Failed to map CDRAM!\n");
        return false;
    }
    
    // Map physically contiguous memory
    if (!createRegion(MemoryRegionType::PHYCONT, 
                      vita::memory::SCE_KERNEL_MAIN_PHYCONT_BASE, 
                      vita::memory::PHYCONT_SIZE, 
                      rw, "PHYCONT")) {
        printf("[MemoryManager] Failed to map physically contiguous memory!\n");
        return false;
    }
    
    // Map shared memory
    if (!createRegion(MemoryRegionType::SHARED, 
                      vita::memory::SCE_KERNEL_MAIN_SHARED_BASE, 
                      vita::memory::SCE_KERNEL_MAIN_SHARED_SIZE, 
                      rw, "Shared Memory")) {
        printf("[MemoryManager] Failed to map shared memory!\n");
        return false;
    }
    
    return true;
}

bool MemoryManager::createRegion(MemoryRegionType type, uint32_t vita_addr, size_t size, 
                                MemoryProtection protection, const std::string& name) {
    // Allocate memory for the region
    uint8_t* host_addr = alignedAlloc(size, 0x1000);
    if (!host_addr) {
        printf("[MemoryManager] Failed to allocate memory for region: %s\n", name.c_str());
        return false;
    }
    
    // Initialize memory to zero
    memset(host_addr, 0, size);
    
    // Create the region
    MemoryRegion region;
    region.type = type;
    region.vita_addr = vita_addr;
    region.host_addr = host_addr;
    region.size = size;
    region.protection = protection;
    region.name = name;
    
    // Add to regions list
    regions.push_back(region);
    usedMemory += size;
    
    printf("[MemoryManager] Mapped region: %s, Vita addr: 0x%08X, Host addr: %p, Size: %zu bytes\n", 
           name.c_str(), vita_addr, host_addr, size);
    
    return true;
}

MemoryRegion* MemoryManager::findRegion(uint32_t vita_addr) {
    for (auto& region : regions) {
        if (vita_addr >= region.vita_addr && vita_addr < region.vita_addr + region.size) {
            return &region;
        }
    }
    return nullptr;
}

uint8_t* MemoryManager::translateAddress(uint32_t vita_addr) {
    std::lock_guard<std::mutex> lock(mutex);
    
    MemoryRegion* region = findRegion(vita_addr);
    if (!region) {
        return nullptr;
    }
    
    uint32_t offset = vita_addr - region->vita_addr;
    return region->host_addr + offset;
}

uint32_t MemoryManager::hostToVita(const void* host_addr) {
    std::lock_guard<std::mutex> lock(mutex);
    
    for (const auto& region : regions) {
        if (host_addr >= region.host_addr && host_addr < region.host_addr + region.size) {
            uint32_t offset = static_cast<uint32_t>(
                reinterpret_cast<const uint8_t*>(host_addr) - region.host_addr);
            return region.vita_addr + offset;
        }
    }
    
    return 0;
}

bool MemoryManager::read_memory(uint32_t addr, void* data, size_t size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Check if the address is valid
    uint8_t* host_addr = translateAddress(addr);
    if (!host_addr) {
        printf("[MemoryManager] Error: Invalid read address 0x%08X\n", addr);
        return false;
    }
    
    // Check memory protection
    if (!checkProtection(addr, size, MemoryAccessType::READ)) {
        printf("[MemoryManager] Error: Read protection violation at 0x%08X\n", addr);
        return false;
    }
    
    // Copy data from host memory
    memcpy(data, host_addr, size);
    
    // Trigger read hooks
    triggerHooks(addr, size, MemoryAccessType::READ, host_addr);
    
    return true;
}

bool MemoryManager::write_memory(uint32_t addr, const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Check if the address is valid
    uint8_t* host_addr = translateAddress(addr);
    if (!host_addr) {
        printf("[MemoryManager] Error: Invalid write address 0x%08X\n", addr);
        return false;
    }
    
    // Check memory protection
    if (!checkProtection(addr, size, MemoryAccessType::WRITE)) {
        printf("[MemoryManager] Error: Write protection violation at 0x%08X\n", addr);
        return false;
    }
    
    // Trigger write hooks before modifying memory
    triggerHooks(addr, size, MemoryAccessType::WRITE, static_cast<const uint8_t*>(data));
    
    // Copy data to host memory
    memcpy(host_addr, data, size);
    
    return true;
}

bool MemoryManager::execute_memory(uint32_t addr, size_t size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Check if the address is valid
    uint8_t* host_addr = translateAddress(addr);
    if (!host_addr) {
        printf("[MemoryManager] Error: Invalid execute address 0x%08X\n", addr);
        return false;
    }
    
    // Check memory protection
    if (!checkProtection(addr, size, MemoryAccessType::EXECUTE)) {
        printf("[MemoryManager] Error: Execute protection violation at 0x%08X\n", addr);
        return false;
    }
    
    // Trigger execute hooks
    triggerHooks(addr, size, MemoryAccessType::EXECUTE, host_addr);
    
    return true;
}

int MemoryManager::addHook(uint32_t addr, size_t size, MemoryAccessType type, MemoryHookCallback callback) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Create a new hook
    MemoryHook hook;
    hook.id = next_hook_id++;
    hook.addr = addr;
    hook.size = size;
    hook.type = type;
    hook.callback = callback;
    
    // Add to hooks list
    hooks.push_back(hook);
    
    printf("[MemoryManager] Added memory hook %d for address 0x%08X, size %zu\n", 
           hook.id, addr, size);
    
    return hook.id;
}

bool MemoryManager::removeHook(int hook_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    for (auto it = hooks.begin(); it != hooks.end(); ++it) {
        if (it->id == hook_id) {
            hooks.erase(it);
            printf("[MemoryManager] Removed memory hook %d\n", hook_id);
            return true;
        }
    }
    
    printf("[MemoryManager] Error: Hook %d not found\n", hook_id);
    return false;
}

void MemoryManager::clearHooks() {
    std::lock_guard<std::mutex> lock(mutex);
    hooks.clear();
    printf("[MemoryManager] Cleared all memory hooks\n");
}

void MemoryManager::triggerHooks(uint32_t addr, size_t size, MemoryAccessType type, const uint8_t* data) {
    // Find and trigger matching hooks
    for (const auto& hook : hooks) {
        // Check if the hook matches the access type
        if (hook.type != type) {
            continue;
        }
        
        // Check if the hook's address range overlaps with the access
        uint32_t hook_end = hook.addr + hook.size;
        uint32_t access_end = addr + size;
        
        if (addr < hook_end && access_end > hook.addr) {
            // Trigger the hook
            hook.callback(addr, size, type, data);
        }
    }
}

bool MemoryManager::checkProtection(uint32_t addr, size_t size, MemoryAccessType type) {
    // Find the region containing the address
    MemoryRegion* region = findRegion(addr);
    if (!region) {
        return false;
    }
    
    // Check if the access crosses region boundaries
    if (addr + size > region->vita_addr + region->size) {
        return false;
    }
    
    // Check protection flags
    switch (type) {
        case MemoryAccessType::READ:
            return region->protection.read;
        case MemoryAccessType::WRITE:
            return region->protection.write;
        case MemoryAccessType::EXECUTE:
            return region->protection.execute;
        default:
            return false;
    }
}

uint32_t MemoryManager::allocate(size_t size, MemoryRegionType type, size_t alignment) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Find the appropriate region for allocation
    uint32_t base_addr = 0;
    switch (type) {
        case MemoryRegionType::RAM:
            base_addr = vita::memory::SCE_KERNEL_MAIN_RAM_BASE;
            break;
        case MemoryRegionType::VRAM:
            base_addr = vita::memory::SCE_KERNEL_MAIN_VRAM_BASE;
            break;
        default:
            printf("[MemoryManager] Error: Unsupported region type for allocation\n");
            return 0;
    }
    
    // Find the region
    MemoryRegion* region = nullptr;
    for (auto& r : regions) {
        if (r.vita_addr == base_addr) {
            region = &r;
            break;
        }
    }
    
    if (!region) {
        printf("[MemoryManager] Error: Region not found for allocation\n");
        return 0;
    }
    
    // Simple first-fit allocation strategy
    // In a real implementation, you'd want a more sophisticated allocator
    uint32_t current_addr = region->vita_addr;
    uint32_t end_addr = region->vita_addr + region->size;
    
    // Align the starting address
    current_addr = (current_addr + alignment - 1) & ~(alignment - 1);
    
    while (current_addr + size <= end_addr) {
        // Check if this range is free
        bool is_free = true;
        for (const auto& alloc : allocations) {
            uint32_t alloc_end = alloc.first + alloc.second;
            if ((current_addr < alloc_end) && (current_addr + size > alloc.first)) {
                is_free = false;
                // Move past this allocation
                current_addr = alloc_end;
                // Re-align
                current_addr = (current_addr + alignment - 1) & ~(alignment - 1);
                break;
            }
        }
        
        if (is_free) {
            // Found a free block
            allocations[current_addr] = size;
            printf("[MemoryManager] Allocated %zu bytes at 0x%08X\n", size, current_addr);
            return current_addr;
        }
    }
    
    printf("[MemoryManager] Error: Out of memory for allocation\n");
    return 0;
}

bool MemoryManager::free(uint32_t addr) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = allocations.find(addr);
    if (it == allocations.end()) {
        printf("[MemoryManager] Error: Invalid address for free: 0x%08X\n", addr);
        return false;
    }
    
    printf("[MemoryManager] Freed %zu bytes at 0x%08X\n", it->second, addr);
    allocations.erase(it);
    return true;
}

bool MemoryManager::protect(uint32_t addr, size_t size, MemoryProtection protection) {
    std::lock_guard<std::mutex> lock(mutex);
    
    MemoryRegion* region = findRegion(addr);
    if (!region) {
        printf("[MemoryManager] Error: Invalid address for protect: 0x%08X\n", addr);
        return false;
    }
    
    // Check if the protection range is within the region
    if (addr + size > region->vita_addr + region->size) {
        printf("[MemoryManager] Error: Protection range exceeds region boundaries\n");
        return false;
    }
    
    // Update protection flags
    // In a real implementation, you'd want to support sub-region protection
    region->protection = protection;
    
    printf("[MemoryManager] Updated protection for region at 0x%08X: r=%d, w=%d, x=%d\n", 
           addr, protection.read, protection.write, protection.execute);
    
    return true;
}

MemoryProtection MemoryManager::getProtection(uint32_t addr) {
    std::lock_guard<std::mutex> lock(mutex);
    
    MemoryRegion* region = findRegion(addr);
    if (!region) {
        return {false, false, false};
    }
    
    return region->protection;
}

size_t MemoryManager::getTotalMemory() const { 
    return totalMemory; 
}

size_t MemoryManager::getUsedMemory() const { 
    return usedMemory; 
}

size_t MemoryManager::getFreeMemory() const { 
    return totalMemory - usedMemory; 
}

void MemoryManager::printMemoryMap() const {
    printf("[MemoryManager] Memory Map:\n");
    for (const auto& region : regions) {
        printf("  Region: %s\n", region.name.c_str());
        printf("    Type: %d\n", (int)region.type);
        printf("    Vita Address: 0x%08X\n", region.vita_addr);
        printf("    Host Address: %p\n", region.host_addr);
        printf("    Size: %zu bytes (%.2f MB)\n", region.size, region.size / (1024.0 * 1024.0));
        printf("    Protection: r=%d, w=%d, x=%d\n", 
               region.protection.read, region.protection.write, region.protection.execute);
    }
    printf("  Total: %zu bytes (%.2f MB)\n", totalMemory, totalMemory / (1024.0 * 1024.0));
    printf("  Used: %zu bytes (%.2f MB)\n", usedMemory, usedMemory / (1024.0 * 1024.0));
    printf("  Free: %zu bytes (%.2f MB)\n", getFreeMemory(), getFreeMemory() / (1024.0 * 1024.0));
}

void MemoryManager::dumpMemory(uint32_t addr, size_t size, const std::string& filename) const {
    // Check if the address is valid
    const uint8_t* host_addr = nullptr;
    for (const auto& region : regions) {
        if (addr >= region.vita_addr && addr < region.vita_addr + region.size) {
            uint32_t offset = addr - region.vita_addr;
            host_addr = region.host_addr + offset;
            break;
        }
    }
    
    if (!host_addr) {
        printf("[MemoryManager] Error: Invalid address for memory dump: 0x%08X\n", addr);
        return;
    }
    
    // Open the file for writing
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        printf("[MemoryManager] Error: Failed to open file for memory dump: %s\n", filename.c_str());
        return;
    }
    
    // Write the memory to the file
    file.write(reinterpret_cast<const char*>(host_addr), size);
    
    printf("[MemoryManager] Dumped %zu bytes from 0x%08X to %s\n", size, addr, filename.c_str());
}

uint8_t* MemoryManager::alignedAlloc(size_t size, size_t alignment) {
#if defined(__SWITCH__)
    // On Switch, use aligned_alloc (libnx)
    return (uint8_t*)aligned_alloc(alignment, size);
#else
    // On other platforms, fallback to posix_memalign
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return nullptr;
    }
    return (uint8_t*)ptr;
#endif
}

void MemoryManager::alignedFree(uint8_t* ptr) {
    std::free(ptr);
}
