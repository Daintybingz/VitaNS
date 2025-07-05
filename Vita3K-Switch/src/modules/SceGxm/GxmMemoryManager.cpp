#include "SceGxm.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>

void* GxmMemoryManager::allocate(size_t size) {
    if (total_allocated + size > max_allocation) {
        printf("[GxmMemoryManager] Out of memory! Requested: %zu, Available: %zu\n", 
               size, max_allocation - total_allocated);
        return nullptr;
    }
    
    void* ptr = malloc(size);
    if (!ptr) {
        printf("[GxmMemoryManager] Failed to allocate %zu bytes\n", size);
        return nullptr;
    }
    
    MemoryBlock block = {ptr, size, false};
    allocated_memory[ptr] = block;
    total_allocated += size;
    return ptr;
}

void* GxmMemoryManager::allocateAligned(size_t size, size_t alignment) {
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        printf("[GxmMemoryManager] Failed to allocate aligned memory\n");
        return nullptr;
    }
    
    MemoryBlock block = {ptr, size, true};
    allocated_memory[ptr] = block;
    total_allocated += size;
    return ptr;
}

void GxmMemoryManager::deallocate(void* ptr) {
    auto it = allocated_memory.find(ptr);
    if (it == allocated_memory.end()) {
        printf("[GxmMemoryManager] Attempt to free unknown pointer\n");
        return;
    }
    
    MemoryBlock block = it->second;
    if (block.is_aligned) {
        free(ptr);
    } else {
        free(ptr);
    }
    
    total_allocated -= block.size;
    allocated_memory.erase(it);
}

void* GxmMemoryManager::allocateHostMemory(size_t size) {
    return allocate(size);
}

void* GxmMemoryManager::allocateParameterBuffer(size_t size) {
    return allocateAligned(size, 4096); // Align to page size
}

void* GxmMemoryManager::allocateCommandBuffer(size_t size) {
    return allocateAligned(size, 4096);
}

void* GxmMemoryManager::allocateRingBuffer(size_t size) {
    return allocateAligned(size, 4096);
}

void* GxmMemoryManager::allocateUsseBuffer(size_t size) {
    return allocateAligned(size, 4096);
}

void* GxmMemoryManager::allocateDriverMemory(size_t size) {
    return allocate(size);
}
