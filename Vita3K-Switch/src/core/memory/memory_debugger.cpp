#include "memory_debugger.h"
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>

// Print a summary of all memory regions
void MemoryDebugger::printMemoryRegionSummary() {
    std::lock_guard<std::mutex> lock(mutex);
    if (!memory_manager) {
        printf("[MemoryDebugger] Error: Memory manager not initialized\n");
        return;
    }
    printf("[MemoryDebugger] Memory Region Summary:\n");
    memory_manager->printMemoryMap();
}

// Singleton instance
MemoryDebugger& MemoryDebugger::getInstance() {
    static MemoryDebugger instance;
    return instance;
}

MemoryDebugger::MemoryDebugger() : memory_manager(nullptr), memory_hook_id(-1), tracking(false) {}

MemoryDebugger::~MemoryDebugger() {
    finalize();
}

bool MemoryDebugger::initialize(MemoryManager* memory_manager) {
    if (!memory_manager) {
        printf("[MemoryDebugger] Error: Invalid memory manager\n");
        return false;
    }
    
    this->memory_manager = memory_manager;
    
    // Register a global memory hook for all memory accesses
    memory_hook_id = memory_manager->addHook(
        0, 0xFFFFFFFF, MemoryAccessType::READ,
        [this](uint32_t addr, size_t size, MemoryAccessType type, const uint8_t* data) {
            this->onMemoryAccess(addr, size, type, data);
        }
    );
    
    if (memory_hook_id < 0) {
        printf("[MemoryDebugger] Error: Failed to register memory hook\n");
        return false;
    }
    
    printf("[MemoryDebugger] Initialized successfully\n");
    return true;
}

void MemoryDebugger::finalize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (memory_manager && memory_hook_id >= 0) {
        memory_manager->removeHook(memory_hook_id);
        memory_hook_id = -1;
    }
    
    breakpoints.clear();
    watches.clear();
    stats.reset();
    tracking = false;
    
    printf("[MemoryDebugger] Finalized\n");
}

int MemoryDebugger::addBreakpoint(uint32_t addr, size_t size, MemoryAccessType type, 
                                const std::string& name, BreakpointCallback callback) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Create a new breakpoint
    MemoryBreakpoint bp;
    bp.id = next_breakpoint_id++;
    bp.addr = addr;
    bp.size = size;
    bp.type = type;
    bp.enabled = true;
    bp.name = name;
    bp.callback = callback;
    
    // Add to breakpoints map
    breakpoints[bp.id] = bp;
    
    printf("[MemoryDebugger] Added breakpoint %d: %s at 0x%08X, size %zu\n", 
           bp.id, name.c_str(), addr, size);
    
    return bp.id;
}

bool MemoryDebugger::removeBreakpoint(int id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = breakpoints.find(id);
    if (it == breakpoints.end()) {
        printf("[MemoryDebugger] Error: Breakpoint %d not found\n", id);
        return false;
    }
    
    printf("[MemoryDebugger] Removed breakpoint %d: %s\n", id, it->second.name.c_str());
    breakpoints.erase(it);
    
    return true;
}

bool MemoryDebugger::enableBreakpoint(int id, bool enable) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = breakpoints.find(id);
    if (it == breakpoints.end()) {
        printf("[MemoryDebugger] Error: Breakpoint %d not found\n", id);
        return false;
    }
    
    it->second.enabled = enable;
    printf("[MemoryDebugger] %s breakpoint %d: %s\n", 
           enable ? "Enabled" : "Disabled", id, it->second.name.c_str());
    
    return true;
}

void MemoryDebugger::clearBreakpoints() {
    std::lock_guard<std::mutex> lock(mutex);
    
    breakpoints.clear();
    printf("[MemoryDebugger] Cleared all breakpoints\n");
}

int MemoryDebugger::addWatch(uint32_t addr, size_t size, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!memory_manager) {
        printf("[MemoryDebugger] Error: Memory manager not initialized\n");
        return -1;
    }
    
    // Create a new watch
    MemoryWatch watch;
    watch.id = next_watch_id++;
    watch.addr = addr;
    watch.size = size;
    watch.name = name;
    watch.enabled = true;
    
    // Read the initial value
    watch.last_value.resize(size);
    if (!memory_manager->read_memory(addr, watch.last_value.data(), size)) {
        printf("[MemoryDebugger] Warning: Failed to read initial value for watch %s\n", name.c_str());
    }
    
    // Add to watches map
    watches[watch.id] = watch;
    
    printf("[MemoryDebugger] Added watch %d: %s at 0x%08X, size %zu\n", 
           watch.id, name.c_str(), addr, size);
    
    return watch.id;
}

bool MemoryDebugger::removeWatch(int id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = watches.find(id);
    if (it == watches.end()) {
        printf("[MemoryDebugger] Error: Watch %d not found\n", id);
        return false;
    }
    
    printf("[MemoryDebugger] Removed watch %d: %s\n", id, it->second.name.c_str());
    watches.erase(it);
    
    return true;
}

bool MemoryDebugger::enableWatch(int id, bool enable) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = watches.find(id);
    if (it == watches.end()) {
        printf("[MemoryDebugger] Error: Watch %d not found\n", id);
        return false;
    }
    
    it->second.enabled = enable;
    printf("[MemoryDebugger] %s watch %d: %s\n", 
           enable ? "Enabled" : "Disabled", id, it->second.name.c_str());
    
    return true;
}

void MemoryDebugger::clearWatches() {
    std::lock_guard<std::mutex> lock(mutex);
    
    watches.clear();
    printf("[MemoryDebugger] Cleared all watches\n");
}

void MemoryDebugger::checkWatches() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!memory_manager) {
        return;
    }
    
    for (auto& pair : watches) {
        MemoryWatch& watch = pair.second;
        
        if (!watch.enabled) {
            continue;
        }
        
        // Read the current value
        std::vector<uint8_t> current_value(watch.size);
        if (!memory_manager->read_memory(watch.addr, current_value.data(), watch.size)) {
            continue;
        }
        
        // Compare with the last value
        if (current_value != watch.last_value) {
            printf("[MemoryDebugger] Watch %d: %s at 0x%08X changed\n", 
                   watch.id, watch.name.c_str(), watch.addr);
            
            // Print the old and new values (for small watches)
            if (watch.size <= 16) {
                printf("  Old value: ");
                for (size_t i = 0; i < watch.size; ++i) {
                    printf("%02X ", watch.last_value[i]);
                }
                printf("\n  New value: ");
                for (size_t i = 0; i < watch.size; ++i) {
                    printf("%02X ", current_value[i]);
                }
                printf("\n");
            }
            
            // Update the last value
            watch.last_value = current_value;
        }
    }
}

void MemoryDebugger::startTracking() {
    std::lock_guard<std::mutex> lock(mutex);
    
    tracking = true;
    printf("[MemoryDebugger] Started memory access tracking\n");
}

void MemoryDebugger::stopTracking() {
    std::lock_guard<std::mutex> lock(mutex);
    
    tracking = false;
    printf("[MemoryDebugger] Stopped memory access tracking\n");
    
    // Print statistics
    printf("[MemoryDebugger] Memory access statistics:\n");
    printf("  Reads: %llu (%zu bytes)\n", stats.reads, stats.total_bytes_read);
    printf("  Writes: %llu (%zu bytes)\n", stats.writes, stats.total_bytes_written);
    printf("  Executes: %llu (%zu bytes)\n", stats.executes, stats.total_bytes_executed);
}

void MemoryDebugger::resetStats() {
    std::lock_guard<std::mutex> lock(mutex);
    
    stats.reset();
    printf("[MemoryDebugger] Reset memory access statistics\n");
}

const MemoryAccessStats& MemoryDebugger::getStats() const {
    return stats;
}

void MemoryDebugger::analyzeMemoryUsage(const std::string& output_file) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!memory_manager) {
        printf("[MemoryDebugger] Error: Memory manager not initialized\n");
        return;
    }
    
    // Open the output file
    std::ofstream file(output_file);
    if (!file) {
        printf("[MemoryDebugger] Error: Failed to open output file: %s\n", output_file.c_str());
        return;
    }
    
    // Write header
    file << "Memory Usage Analysis\n";
    file << "====================\n\n";
    
    // Write memory regions
    file << "Memory Regions:\n";
    file << "--------------\n";
    
    // Get memory map information from memory manager
    // This is a simplified version - in a real implementation, you'd want to get more detailed information
    file << "Total Memory: " << memory_manager->getTotalMemory() << " bytes\n";
    file << "Used Memory: " << memory_manager->getUsedMemory() << " bytes\n";
    file << "Free Memory: " << memory_manager->getFreeMemory() << " bytes\n\n";
    
    // Write memory access statistics
    file << "Memory Access Statistics:\n";
    file << "------------------------\n";
    file << "Reads: " << stats.reads << " (" << stats.total_bytes_read << " bytes)\n";
    file << "Writes: " << stats.writes << " (" << stats.total_bytes_written << " bytes)\n";
    file << "Executes: " << stats.executes << " (" << stats.total_bytes_executed << " bytes)\n\n";
    
    // Close the file
    file.close();
    
    printf("[MemoryDebugger] Memory usage analysis written to %s\n", output_file.c_str());
}

void MemoryDebugger::findPattern(const std::vector<uint8_t>& pattern, uint32_t start_addr, size_t search_size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!memory_manager) {
        printf("[MemoryDebugger] Error: Memory manager not initialized\n");
        return;
    }
    
    if (pattern.empty()) {
        printf("[MemoryDebugger] Error: Empty pattern\n");
        return;
    }
    
    // Read the memory to search
    std::vector<uint8_t> memory(search_size);
    if (!memory_manager->read_memory(start_addr, memory.data(), search_size)) {
        printf("[MemoryDebugger] Error: Failed to read memory for pattern search\n");
        return;
    }
    
    // Search for the pattern
    size_t matches = 0;
    for (size_t i = 0; i <= memory.size() - pattern.size(); ++i) {
        bool found = true;
        for (size_t j = 0; j < pattern.size(); ++j) {
            if (memory[i + j] != pattern[j]) {
                found = false;
                break;
            }
        }
        
        if (found) {
            uint32_t addr = start_addr + i;
            printf("[MemoryDebugger] Pattern found at 0x%08X\n", addr);
            matches++;
        }
    }
    
    printf("[MemoryDebugger] Pattern search complete: %zu matches found\n", matches);
}

void MemoryDebugger::dumpMemory(uint32_t addr, size_t size, const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!memory_manager) {
        printf("[MemoryDebugger] Error: Memory manager not initialized\n");
        return;
    }
    
    memory_manager->dumpMemory(addr, size, filename);
}

void MemoryDebugger::hexDump(uint32_t addr, size_t size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!memory_manager) {
        printf("[MemoryDebugger] Error: Memory manager not initialized\n");
        return;
    }
    
    // Read the memory to dump
    std::vector<uint8_t> memory(size);
    if (!memory_manager->read_memory(addr, memory.data(), size)) {
        printf("[MemoryDebugger] Error: Failed to read memory for hex dump\n");
        return;
    }
    
    // Print the hex dump
    printf("[MemoryDebugger] Hex dump of 0x%08X - 0x%08X (%zu bytes):\n", addr, addr + size - 1, size);
    
    const size_t BYTES_PER_LINE = 16;
    
    for (size_t i = 0; i < size; i += BYTES_PER_LINE) {
        // Print address
        printf("0x%08X: ", addr + i);
        
        // Print hex values
        for (size_t j = 0; j < BYTES_PER_LINE; ++j) {
            if (i + j < size) {
                printf("%02X ", memory[i + j]);
            } else {
                printf("   ");
            }
        }
        
        // Print ASCII representation
        printf(" | ");
        for (size_t j = 0; j < BYTES_PER_LINE; ++j) {
            if (i + j < size) {
                uint8_t c = memory[i + j];
                printf("%c", (c >= 32 && c <= 126) ? c : '.');
            } else {
                printf(" ");
            }
        }
        
        printf("\n");
    }
}

void MemoryDebugger::onMemoryAccess(uint32_t addr, size_t size, MemoryAccessType type, const uint8_t* data) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Update statistics if tracking is enabled
    if (tracking) {
        switch (type) {
            case MemoryAccessType::READ:
                stats.reads++;
                stats.total_bytes_read += size;
                break;
            case MemoryAccessType::WRITE:
                stats.writes++;
                stats.total_bytes_written += size;
                break;
            case MemoryAccessType::EXECUTE:
                stats.executes++;
                stats.total_bytes_executed += size;
                break;
        }
    }
    
    // Check breakpoints
    for (const auto& pair : breakpoints) {
        const MemoryBreakpoint& bp = pair.second;
        
        if (!bp.enabled || bp.type != type) {
            continue;
        }
        
        // Check if the access overlaps with the breakpoint
        uint32_t bp_end = bp.addr + bp.size;
        uint32_t access_end = addr + size;
        
        if (addr < bp_end && access_end > bp.addr) {
            printf("[MemoryDebugger] Breakpoint %d: %s triggered at 0x%08X\n", 
                   bp.id, bp.name.c_str(), addr);
            
            // Call the breakpoint callback if provided
            if (bp.callback) {
                bp.callback(addr, size, type, data);
            }
        }
    }
}
