#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <set>
#include <memory>
#include "block_analyzer.h"

// Register usage information
struct RegisterUsage {
    bool read;          // Register is read
    bool written;       // Register is written
    bool first_read;    // First use is a read
    bool last_write;    // Last use is a write
    uint32_t first_use; // Index of first use instruction
    uint32_t last_use;  // Index of last use instruction
    
    RegisterUsage() : read(false), written(false), first_read(false), last_write(false), first_use(0), last_use(0) {}
};

// Register allocation information
struct RegisterAllocation {
    uint8_t vita_reg;   // PS Vita register
    uint8_t switch_reg; // Nintendo Switch register
    bool spilled;       // Whether the register is spilled to memory
    uint32_t spill_slot; // Memory slot for spilled register
    
    RegisterAllocation() : vita_reg(0), switch_reg(0), spilled(false), spill_slot(0) {}
};

// Register allocator
class RegisterAllocator {
public:
    // Register constants
    static const uint8_t PC_REG = 15;
    static const uint8_t LR_REG = 14;
    static const uint8_t SP_REG = 13;
    
    RegisterAllocator();
    ~RegisterAllocator();
    
    // Analyze register usage in a basic block
    void analyzeRegisterUsage(const BasicBlock& block);
    
    // Allocate registers for a basic block
    std::vector<RegisterAllocation> allocateRegisters(const BasicBlock& block);
    
    // Get register allocation for a specific instruction
    std::vector<RegisterAllocation> getInstructionAllocation(uint32_t inst_index);
    
    // Get register usage statistics
    void getRegisterUsageStats(uint32_t& total_reads, uint32_t& total_writes, 
                              uint32_t& spills, uint32_t& reloads);
    
private:
    // Register usage for each ARM register
    std::vector<RegisterUsage> register_usage;
    
    // Register allocations for each instruction
    std::vector<std::vector<RegisterAllocation>> instruction_allocations;
    
    // Register usage statistics
    uint32_t total_reads;
    uint32_t total_writes;
    uint32_t spills;
    uint32_t reloads;
    
    // Helper methods
    void analyzeInstructionRegisterUsage(const ARMInstruction& inst, uint32_t inst_index);
    void buildInterferenceGraph();
    void allocateRegistersLinearScan();
    
    // Register sets
    static const uint8_t NUM_VITA_REGS = 16;
    static const uint8_t NUM_SWITCH_REGS = 16;
    
    // Interference graph
    std::vector<std::set<uint8_t>> interference_graph;
    
    // Live ranges
    struct LiveRange {
        uint8_t reg;
        uint32_t start;
        uint32_t end;
        
        LiveRange(uint8_t r, uint32_t s, uint32_t e) : reg(r), start(s), end(e) {}
        
        // Sort by start position
        bool operator<(const LiveRange& other) const {
            return start < other.start;
        }
    };
    std::vector<LiveRange> live_ranges;
};

// Register allocation optimizer
class RegisterAllocationOptimizer {
public:
    RegisterAllocationOptimizer();
    ~RegisterAllocationOptimizer();
    
    // Optimize register allocation for a function
    void optimizeFunction(const std::vector<std::unique_ptr<BasicBlock>>& blocks);
    
    // Get optimized register allocation for a basic block
    std::vector<RegisterAllocation> getBlockAllocation(uint32_t block_address);
    
    // Get register allocation statistics
    void getAllocationStats(uint32_t& total_blocks, uint32_t& optimized_blocks, 
                           uint32_t& total_spills, uint32_t& total_reloads);
    
private:
    // Register allocator for each basic block
    std::unordered_map<uint32_t, std::unique_ptr<RegisterAllocator>> block_allocators;
    
    // Global register allocation
    std::unordered_map<uint8_t, uint8_t> global_allocation;
    
    // Statistics
    uint32_t total_blocks;
    uint32_t optimized_blocks;
    uint32_t total_spills;
    uint32_t total_reloads;
    
    // Helper methods
    void analyzeGlobalRegisterUsage(const std::vector<std::unique_ptr<BasicBlock>>& blocks);
    void assignGlobalRegisters();
    void optimizeBlockBoundaries(const std::vector<std::unique_ptr<BasicBlock>>& blocks);
};
