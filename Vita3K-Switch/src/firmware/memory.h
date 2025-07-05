#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <mutex>

namespace firmware {

// Memory permission flags
enum class MemoryPermission : uint32_t {
    NONE = 0,
    READ = 1 << 0,
    WRITE = 1 << 1,
    EXECUTE = 1 << 2,
    READ_WRITE = READ | WRITE,
    READ_EXECUTE = READ | EXECUTE,
    READ_WRITE_EXECUTE = READ | WRITE | EXECUTE
};

// Memory type
enum class MemoryType : uint32_t {
    UNKNOWN,
    RAM,
    VRAM,
    KERNEL,
    USER,
    SHARED
};

// Memory block
struct MemoryBlock {
    uint32_t id;
    uint32_t process_id;
    uint64_t base_address;
    uint64_t size;
    MemoryPermission permission;
    MemoryType type;
    std::string name;
    bool allocated;
    void* host_address;
};

// Memory manager class
class MemoryManager {
public:
    MemoryManager();
    ~MemoryManager();

    // Initialize the memory manager
    bool initialize(uint64_t total_memory_size);

    // Allocate a memory block
    std::shared_ptr<MemoryBlock> allocate_memory(uint32_t process_id, uint64_t size, MemoryPermission permission, MemoryType type, const std::string &name);

    // Free a memory block
    bool free_memory(uint32_t block_id);

    // Map a memory block
    bool map_memory(std::shared_ptr<MemoryBlock> block, uint64_t address);

    // Unmap a memory block
    bool unmap_memory(std::shared_ptr<MemoryBlock> block);

    // Change memory permission
    bool change_permission(std::shared_ptr<MemoryBlock> block, MemoryPermission permission);

    // Get a memory block by ID
    std::shared_ptr<MemoryBlock> get_block_by_id(uint32_t block_id);

    // Get a memory block by address
    std::shared_ptr<MemoryBlock> get_block_by_address(uint64_t address);

    // Get all memory blocks for a process
    std::vector<std::shared_ptr<MemoryBlock>> get_blocks_by_process(uint32_t process_id);

    // Read memory
    template <typename T>
    bool read_memory(uint64_t address, T &value) {
        auto block = get_block_by_address(address);
        if (!block) {
            return false;
        }

        if (!(static_cast<uint32_t>(block->permission) & static_cast<uint32_t>(MemoryPermission::READ))) {
            return false;
        }

        uint64_t offset = address - block->base_address;
        if (offset + sizeof(T) > block->size) {
            return false;
        }

        value = *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(block->host_address) + offset);
        return true;
    }

    // Write memory
    template <typename T>
    bool write_memory(uint64_t address, const T &value) {
        auto block = get_block_by_address(address);
        if (!block) {
            return false;
        }

        if (!(static_cast<uint32_t>(block->permission) & static_cast<uint32_t>(MemoryPermission::WRITE))) {
            return false;
        }

        uint64_t offset = address - block->base_address;
        if (offset + sizeof(T) > block->size) {
            return false;
        }

        *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(block->host_address) + offset) = value;
        return true;
    }

    // Read memory block
    bool read_memory_block(uint64_t address, void *data, uint64_t size);

    // Write memory block
    bool write_memory_block(uint64_t address, const void *data, uint64_t size);

    // Check if an address is valid
    bool is_valid_address(uint64_t address);

    // Check if an address range is valid
    bool is_valid_range(uint64_t address, uint64_t size);

    // Check if an address has a specific permission
    bool has_permission(uint64_t address, MemoryPermission permission);

    // Get total memory size
    uint64_t get_total_memory_size() const;

    // Get used memory size
    uint64_t get_used_memory_size() const;

    // Get free memory size
    uint64_t get_free_memory_size() const;

    // Dump memory map
    void dump_memory_map() const;

private:
    // Find a free memory region
    uint64_t find_free_region(uint64_t size);

    // Total memory size
    uint64_t total_memory_size;

    // Used memory size
    uint64_t used_memory_size;

    // Memory blocks by ID
    std::map<uint32_t, std::shared_ptr<MemoryBlock>> blocks_by_id;

    // Memory blocks by address
    std::map<uint64_t, std::shared_ptr<MemoryBlock>> blocks_by_address;

    // Memory blocks by process
    std::map<uint32_t, std::vector<std::shared_ptr<MemoryBlock>>> blocks_by_process;

    // Next block ID
    uint32_t next_block_id;

    // Base address for memory allocation
    uint64_t base_address;

    // Host memory
    void* host_memory;

    // Mutex for thread safety
    mutable std::mutex mutex;
};

// Memory protection exception
class MemoryProtectionException : public std::exception {
public:
    MemoryProtectionException(uint64_t address, MemoryPermission required_permission);
    virtual ~MemoryProtectionException() = default;

    const char* what() const noexcept override;
    uint64_t get_address() const;
    MemoryPermission get_required_permission() const;

private:
    uint64_t address;
    MemoryPermission required_permission;
    mutable std::string message;
};

// Memory access exception
class MemoryAccessException : public std::exception {
public:
    MemoryAccessException(uint64_t address);
    virtual ~MemoryAccessException() = default;

    const char* what() const noexcept override;
    uint64_t get_address() const;

private:
    uint64_t address;
    mutable std::string message;
};

} // namespace firmware
