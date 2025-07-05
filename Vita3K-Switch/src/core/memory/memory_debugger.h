#pragma once

#include "memory_manager.h"
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>

// Forward declarations
class MemoryDebugger;

// Breakpoint callback type
using BreakpointCallback = std::function<void(uint32_t addr, size_t size, MemoryAccessType type, const uint8_t* data)>;

// Memory access statistics
struct MemoryAccessStats {
    uint64_t reads = 0;
    uint64_t writes = 0;
    uint64_t executes = 0;
    size_t total_bytes_read = 0;
    size_t total_bytes_written = 0;
    size_t total_bytes_executed = 0;
    
    void reset() {
        reads = 0;
        writes = 0;
        executes = 0;
        total_bytes_read = 0;
        total_bytes_written = 0;
        total_bytes_executed = 0;
    }
};

// Memory breakpoint
struct MemoryBreakpoint {
    int id;
    uint32_t addr;
    size_t size;
    MemoryAccessType type;
    bool enabled;
    std::string name;
    BreakpointCallback callback;
};

// Memory watch
struct MemoryWatch {
    int id;
    uint32_t addr;
    size_t size;
    std::string name;
    bool enabled;
    std::vector<uint8_t> last_value;
};

class MemoryDebugger {
public:
    // Singleton pattern
    static MemoryDebugger& getInstance();
    
    // Initialize and finalize
    bool initialize(MemoryManager* memory_manager);
    void finalize();

    // --- Memory Analysis Tools ---
    // Dump a memory region to a binary file
    void dumpMemory(uint32_t addr, size_t size, const std::string& filename);
    // Hex dump a memory region to stdout
    void hexDump(uint32_t addr, size_t size);
    // Search for a pattern in memory
    void findPattern(const std::vector<uint8_t>& pattern, uint32_t start_addr, size_t search_size);
    // Analyze memory usage and write a summary to a file
    void analyzeMemoryUsage(const std::string& output_file);
    // Print a summary of all memory regions to stdout
    void printMemoryRegionSummary();

    // Breakpoint management
    int addBreakpoint(uint32_t addr, size_t size, MemoryAccessType type, 
                     const std::string& name, BreakpointCallback callback = nullptr);
    bool removeBreakpoint(int id);
    bool enableBreakpoint(int id, bool enable);
    void clearBreakpoints();
    
    // Memory watch management
    int addWatch(uint32_t addr, size_t size, const std::string& name);
    bool removeWatch(int id);
    bool enableWatch(int id, bool enable);
    void clearWatches();
    
    // Check for watch changes
    void checkWatches();
    
    // Memory access tracking
    void startTracking();
    void stopTracking();
    void resetStats();
    const MemoryAccessStats& getStats() const;
    
private:
    MemoryDebugger();
    ~MemoryDebugger();
    MemoryDebugger(const MemoryDebugger&) = delete;
    MemoryDebugger& operator=(const MemoryDebugger&) = delete;
    
    // Memory hook callback
    void onMemoryAccess(uint32_t addr, size_t size, MemoryAccessType type, const uint8_t* data);
    
    // Internal state
    MemoryManager* memory_manager = nullptr;
    int memory_hook_id = -1;
    
    // Breakpoints
    std::map<int, MemoryBreakpoint> breakpoints;
    int next_breakpoint_id = 1;
    
    // Watches
    std::map<int, MemoryWatch> watches;
    int next_watch_id = 1;
    
    // Statistics
    MemoryAccessStats stats;
    bool tracking = false;
    
    // Thread safety
    std::mutex mutex;
};
