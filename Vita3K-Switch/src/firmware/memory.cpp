#include "memory.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>

namespace firmware {

// Memory manager implementation
MemoryManager::MemoryManager()
    : total_memory_size(0), used_memory_size(0), next_block_id(1), base_address(0x80000000), host_memory(nullptr) {
}

MemoryManager::~MemoryManager() {
    // Free all memory blocks
    for (auto &pair : blocks_by_id) {
        if (pair.second->allocated) {
            // In a real implementation, we would free the memory here
        }
    }

    // Free host memory
    if (host_memory) {
        free(host_memory);
        host_memory = nullptr;
    }
}

bool MemoryManager::initialize(uint64_t total_memory_size) {
    std::lock_guard<std::mutex> lock(mutex);

    if (host_memory) {
        std::cerr << "Memory manager is already initialized" << std::endl;
        return false;
    }

    std::cout << "Initializing memory manager with " << (total_memory_size / (1024 * 1024)) << " MB of memory" << std::endl;

    this->total_memory_size = total_memory_size;
    this->used_memory_size = 0;

    // Allocate host memory
    host_memory = malloc(total_memory_size);
    if (!host_memory) {
        std::cerr << "Failed to allocate host memory" << std::endl;
        return false;
    }

    // Clear memory
    memset(host_memory, 0, total_memory_size);

    std::cout << "Memory manager initialized successfully" << std::endl;
    return true;
}

std::shared_ptr<MemoryBlock> MemoryManager::allocate_memory(uint32_t process_id, uint64_t size, MemoryPermission permission, MemoryType type, const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex);

    if (!host_memory) {
        std::cerr << "Memory manager is not initialized" << std::endl;
        return nullptr;
    }

    if (size == 0) {
        std::cerr << "Cannot allocate zero-sized memory block" << std::endl;
        return nullptr;
    }

    if (used_memory_size + size > total_memory_size) {
        std::cerr << "Not enough memory to allocate " << size << " bytes" << std::endl;
        return nullptr;
    }

    // Align size to 4KB
    size = (size + 0xFFF) & ~0xFFF;

    // Find a free memory region
    uint64_t address = find_free_region(size);
    if (address == 0) {
        std::cerr << "Failed to find a free memory region of size " << size << std::endl;
        return nullptr;
    }

    // Create memory block
    auto block = std::make_shared<MemoryBlock>();
    block->id = next_block_id++;
    block->process_id = process_id;
    block->base_address = address;
    block->size = size;
    block->permission = permission;
    block->type = type;
    block->name = name;
    block->allocated = true;
    
    // Calculate host address
    uint64_t offset = address - base_address;
    block->host_address = reinterpret_cast<uint8_t*>(host_memory) + offset;

    // Register block
    blocks_by_id[block->id] = block;
    blocks_by_address[block->base_address] = block;
    blocks_by_process[process_id].push_back(block);

    // Update used memory size
    used_memory_size += size;

    std::cout << "Allocated memory block " << block->id << " at 0x" << std::hex << block->base_address 
              << " of size 0x" << block->size << std::dec << " for process " << process_id << std::endl;
    return block;
}

bool MemoryManager::free_memory(uint32_t block_id) {
    std::lock_guard<std::mutex> lock(mutex);

    auto it = blocks_by_id.find(block_id);
    if (it == blocks_by_id.end()) {
        std::cerr << "Memory block " << block_id << " not found" << std::endl;
        return false;
    }

    auto block = it->second;
    if (!block->allocated) {
        std::cerr << "Memory block " << block_id << " is not allocated" << std::endl;
        return false;
    }

    // Clear memory
    memset(block->host_address, 0, block->size);

    // Update used memory size
    used_memory_size -= block->size;

    // Unregister block
    blocks_by_address.erase(block->base_address);
    
    // Remove from process blocks
    auto &process_blocks = blocks_by_process[block->process_id];
    process_blocks.erase(std::remove_if(process_blocks.begin(), process_blocks.end(),
        [block_id](const std::shared_ptr<MemoryBlock> &b) { return b->id == block_id; }),
        process_blocks.end());
    
    // Mark as not allocated
    block->allocated = false;

    std::cout << "Freed memory block " << block_id << " at 0x" << std::hex << block->base_address 
              << " of size 0x" << block->size << std::dec << " for process " << block->process_id << std::endl;
    return true;
}

bool MemoryManager::map_memory(std::shared_ptr<MemoryBlock> block, uint64_t address) {
    std::lock_guard<std::mutex> lock(mutex);

    if (!block) {
        std::cerr << "Invalid memory block" << std::endl;
        return false;
    }

    if (!block->allocated) {
        std::cerr << "Memory block " << block->id << " is not allocated" << std::endl;
        return false;
    }

    // Check if the address is already mapped
    if (blocks_by_address.find(address) != blocks_by_address.end()) {
        std::cerr << "Address 0x" << std::hex << address << std::dec << " is already mapped" << std::endl;
        return false;
    }

    // Unmap from old address
    blocks_by_address.erase(block->base_address);

    // Update block address
    block->base_address = address;
    
    // Calculate host address
    uint64_t offset = address - base_address;
    block->host_address = reinterpret_cast<uint8_t*>(host_memory) + offset;

    // Map to new address
    blocks_by_address[address] = block;

    std::cout << "Mapped memory block " << block->id << " to 0x" << std::hex << address << std::dec << std::endl;
    return true;
}

bool MemoryManager::unmap_memory(std::shared_ptr<MemoryBlock> block) {
    std::lock_guard<std::mutex> lock(mutex);

    if (!block) {
        std::cerr << "Invalid memory block" << std::endl;
        return false;
    }

    if (!block->allocated) {
        std::cerr << "Memory block " << block->id << " is not allocated" << std::endl;
        return false;
    }

    // Unmap from address
    blocks_by_address.erase(block->base_address);

    std::cout << "Unmapped memory block " << block->id << " from 0x" << std::hex << block->base_address << std::dec << std::endl;
    return true;
}

bool MemoryManager::change_permission(std::shared_ptr<MemoryBlock> block, MemoryPermission permission) {
    std::lock_guard<std::mutex> lock(mutex);

    if (!block) {
        std::cerr << "Invalid memory block" << std::endl;
        return false;
    }

    if (!block->allocated) {
        std::cerr << "Memory block " << block->id << " is not allocated" << std::endl;
        return false;
    }

    // Update permission
    block->permission = permission;

    std::cout << "Changed permission of memory block " << block->id << " to " 
              << static_cast<uint32_t>(permission) << std::endl;
    return true;
}

std::shared_ptr<MemoryBlock> MemoryManager::get_block_by_id(uint32_t block_id) {
    std::lock_guard<std::mutex> lock(mutex);

    auto it = blocks_by_id.find(block_id);
    if (it == blocks_by_id.end()) {
        return nullptr;
    }

    return it->second;
}

std::shared_ptr<MemoryBlock> MemoryManager::get_block_by_address(uint64_t address) {
    std::lock_guard<std::mutex> lock(mutex);

    // Find the block with the largest base address that is less than or equal to the given address
    auto it = blocks_by_address.upper_bound(address);
    if (it == blocks_by_address.begin()) {
        return nullptr;
    }
    
    --it;
    auto block = it->second;
    
    // Check if the address is within the block
    if (address >= block->base_address && address < block->base_address + block->size) {
        return block;
    }
    
    return nullptr;
}

std::vector<std::shared_ptr<MemoryBlock>> MemoryManager::get_blocks_by_process(uint32_t process_id) {
    std::lock_guard<std::mutex> lock(mutex);

    auto it = blocks_by_process.find(process_id);
    if (it == blocks_by_process.end()) {
        return {};
    }

    return it->second;
}

bool MemoryManager::read_memory_block(uint64_t address, void *data, uint64_t size) {
    if (!data) {
        std::cerr << "Invalid data pointer" << std::endl;
        return false;
    }

    if (size == 0) {
        return true;
    }

    // Check if the address range is valid
    if (!is_valid_range(address, size)) {
        std::cerr << "Invalid address range: 0x" << std::hex << address << " - 0x" 
                  << (address + size) << std::dec << std::endl;
        return false;
    }

    // Check if the address has read permission
    if (!has_permission(address, MemoryPermission::READ)) {
        std::cerr << "Address 0x" << std::hex << address << std::dec 
                  << " does not have read permission" << std::endl;
        return false;
    }

    // Get the memory block
    auto block = get_block_by_address(address);
    if (!block) {
        std::cerr << "No memory block found at address 0x" << std::hex << address << std::dec << std::endl;
        return false;
    }

    // Calculate offset within the block
    uint64_t offset = address - block->base_address;

    // Copy data
    memcpy(data, reinterpret_cast<uint8_t*>(block->host_address) + offset, size);
    return true;
}

bool MemoryManager::write_memory_block(uint64_t address, const void *data, uint64_t size) {
    if (!data) {
        std::cerr << "Invalid data pointer" << std::endl;
        return false;
    }

    if (size == 0) {
        return true;
    }

    // Check if the address range is valid
    if (!is_valid_range(address, size)) {
        std::cerr << "Invalid address range: 0x" << std::hex << address << " - 0x" 
                  << (address + size) << std::dec << std::endl;
        return false;
    }

    // Check if the address has write permission
    if (!has_permission(address, MemoryPermission::WRITE)) {
        std::cerr << "Address 0x" << std::hex << address << std::dec 
                  << " does not have write permission" << std::endl;
        return false;
    }

    // Get the memory block
    auto block = get_block_by_address(address);
    if (!block) {
        std::cerr << "No memory block found at address 0x" << std::hex << address << std::dec << std::endl;
        return false;
    }

    // Calculate offset within the block
    uint64_t offset = address - block->base_address;

    // Copy data
    memcpy(reinterpret_cast<uint8_t*>(block->host_address) + offset, data, size);
    return true;
}

bool MemoryManager::is_valid_address(uint64_t address) {
    std::lock_guard<std::mutex> lock(mutex);

    // Find the block with the largest base address that is less than or equal to the given address
    auto it = blocks_by_address.upper_bound(address);
    if (it == blocks_by_address.begin()) {
        return false;
    }
    
    --it;
    auto block = it->second;
    
    // Check if the address is within the block
    return (address >= block->base_address && address < block->base_address + block->size);
}

bool MemoryManager::is_valid_range(uint64_t address, uint64_t size) {
    if (size == 0) {
        return true;
    }

    // Check if the start address is valid
    if (!is_valid_address(address)) {
        return false;
    }

    // Check if the end address is valid
    return is_valid_address(address + size - 1);
}

bool MemoryManager::has_permission(uint64_t address, MemoryPermission permission) {
    auto block = get_block_by_address(address);
    if (!block) {
        return false;
    }

    // Check if the block has the required permission
    return (static_cast<uint32_t>(block->permission) & static_cast<uint32_t>(permission)) != 0;
}

uint64_t MemoryManager::get_total_memory_size() const {
    return total_memory_size;
}

uint64_t MemoryManager::get_used_memory_size() const {
    return used_memory_size;
}

uint64_t MemoryManager::get_free_memory_size() const {
    return total_memory_size - used_memory_size;
}

void MemoryManager::dump_memory_map() const {
    std::lock_guard<std::mutex> lock(mutex);

    std::cout << "Memory Map:" << std::endl;
    std::cout << "Total Memory: " << (total_memory_size / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "Used Memory: " << (used_memory_size / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "Free Memory: " << ((total_memory_size - used_memory_size) / (1024 * 1024)) << " MB" << std::endl;
    std::cout << "Blocks: " << blocks_by_id.size() << std::endl;

    std::cout << std::setw(10) << "ID" << " | "
              << std::setw(10) << "Process" << " | "
              << std::setw(18) << "Base Address" << " | "
              << std::setw(18) << "Size" << " | "
              << std::setw(10) << "Perm" << " | "
              << std::setw(10) << "Type" << " | "
              << "Name" << std::endl;
    
    std::cout << std::string(100, '-') << std::endl;

    for (const auto &pair : blocks_by_id) {
        const auto &block = pair.second;
        if (!block->allocated) {
            continue;
        }

        std::cout << std::setw(10) << block->id << " | "
                  << std::setw(10) << block->process_id << " | "
                  << "0x" << std::hex << std::setw(16) << block->base_address << " | "
                  << "0x" << std::setw(16) << block->size << " | "
                  << std::dec << std::setw(10) << static_cast<uint32_t>(block->permission) << " | "
                  << std::setw(10) << static_cast<uint32_t>(block->type) << " | "
                  << block->name << std::endl;
    }
}

uint64_t MemoryManager::find_free_region(uint64_t size) {
    // Align size to 4KB
    size = (size + 0xFFF) & ~0xFFF;

    // If there are no blocks, start from base address
    if (blocks_by_address.empty()) {
        return base_address;
    }

    // Find a free region between blocks
    uint64_t current_address = base_address;
    for (const auto &pair : blocks_by_address) {
        const auto &block = pair.second;
        
        // Check if there is enough space before this block
        if (block->base_address - current_address >= size) {
            return current_address;
        }
        
        // Move to the end of this block
        current_address = block->base_address + block->size;
        
        // Align to 4KB
        current_address = (current_address + 0xFFF) & ~0xFFF;
    }

    // Check if there is enough space after the last block
    if (base_address + total_memory_size - current_address >= size) {
        return current_address;
    }

    // No free region found
    return 0;
}

// Memory protection exception implementation
MemoryProtectionException::MemoryProtectionException(uint64_t address, MemoryPermission required_permission)
    : address(address), required_permission(required_permission) {
}

const char* MemoryProtectionException::what() const noexcept {
    std::ostringstream oss;
    oss << "Memory protection exception at address 0x" << std::hex << address
        << ": required permission " << static_cast<uint32_t>(required_permission) << std::dec;
    message = oss.str();
    return message.c_str();
}

uint64_t MemoryProtectionException::get_address() const {
    return address;
}

MemoryPermission MemoryProtectionException::get_required_permission() const {
    return required_permission;
}

// Memory access exception implementation
MemoryAccessException::MemoryAccessException(uint64_t address)
    : address(address) {
}

const char* MemoryAccessException::what() const noexcept {
    std::ostringstream oss;
    oss << "Memory access exception at address 0x" << std::hex << address << std::dec;
    message = oss.str();
    return message.c_str();
}

uint64_t MemoryAccessException::get_address() const {
    return address;
}

} // namespace firmware
