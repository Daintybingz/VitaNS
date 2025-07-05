#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include "block_analyzer.h"

// Memory access types
enum class MemoryAccessType {
    LOAD,           // Load from memory
    STORE,          // Store to memory
    LOAD_MULTIPLE,  // Load multiple registers (LDM)
    STORE_MULTIPLE, // Store multiple registers (STM)
    EXCLUSIVE_LOAD, // Exclusive load (LDREX)
    EXCLUSIVE_STORE // Exclusive store (STREX)
};

// Memory access information
struct MemoryAccess {
    uint32_t address;          // Memory address (if constant)
    uint8_t base_reg;          // Base register
    int32_t offset;            // Offset from base register
    uint8_t index_reg;         // Index register (if used)
    uint8_t shift_type;        // Shift type for index register
    uint8_t shift_amount;      // Shift amount for index register
    uint8_t data_reg;          // Data register
    uint8_t size;              // Access size in bytes (1, 2, 4, 8)
    bool is_signed;            // Whether the load is sign-extended
    bool is_writeback;         // Whether the base register is updated
    bool is_pre_indexed;       // Whether the offset is applied before access
    MemoryAccessType type;     // Type of memory access
    std::vector<uint8_t> regs; // Registers for LDM/STM
    
    MemoryAccess() 
        : address(0), base_reg(0), offset(0), index_reg(0), shift_type(0), 
          shift_amount(0), data_reg(0), size(0), is_signed(false), 
          is_writeback(false), is_pre_indexed(false), type(MemoryAccessType::LOAD) {}
};

// Memory access optimization
struct MemoryAccessOptimization {
    bool use_direct_address;   // Use direct address instead of base+offset
    bool use_switch_address;   // Use Switch address mapping
    bool combine_accesses;     // Combine multiple accesses
    bool use_wider_access;     // Use wider access (e.g., 32-bit instead of 8-bit)
    bool eliminate_access;     // Eliminate redundant access
    uint32_t direct_address;   // Direct address (if use_direct_address)
    uint32_t switch_address;   // Switch address (if use_switch_address)
    
    MemoryAccessOptimization() 
        : use_direct_address(false), use_switch_address(false), 
          combine_accesses(false), use_wider_access(false), 
          eliminate_access(false), direct_address(0), switch_address(0) {}
};

// Memory access optimizer
class MemoryAccessOptimizer {
public:
    MemoryAccessOptimizer();
    ~MemoryAccessOptimizer();
    
    // Analyze memory accesses in a basic block
    void analyzeMemoryAccesses(const BasicBlock& block);
    
    // Optimize memory accesses in a basic block
    std::vector<std::pair<MemoryAccess, MemoryAccessOptimization>> optimizeMemoryAccesses();
    
    // Get memory access statistics
    void getMemoryAccessStats(uint32_t& total_loads, uint32_t& total_stores, 
                             uint32_t& optimized_loads, uint32_t& optimized_stores);
    
private:
    // Memory accesses in the block
    std::vector<MemoryAccess> memory_accesses;
    
    // Memory access optimizations
    std::vector<MemoryAccessOptimization> optimizations;
    
    // Memory access statistics
    uint32_t total_loads;
    uint32_t total_stores;
    uint32_t optimized_loads;
    uint32_t optimized_stores;
    
    // Helper methods
    void analyzeInstructionMemoryAccess(const ARMInstruction& inst);
    void optimizeConstantAddresses();
    void optimizeAddressTranslation();
    void optimizeCombineAccesses();
    void optimizeAccessWidth();
    void optimizeRedundantAccesses();
    
    // Memory address translation
    uint32_t translateVitaToSwitchAddress(uint32_t vita_address);
};

// Memory access translation cache
class MemoryAccessTranslationCache {
public:
    MemoryAccessTranslationCache();
    ~MemoryAccessTranslationCache();
    
    // Add a translation to the cache
    void addTranslation(uint32_t vita_address, uint32_t switch_address);
    
    // Get a translation from the cache
    bool getTranslation(uint32_t vita_address, uint32_t& switch_address);
    
    // Clear the cache
    void clear();
    
    // Get cache statistics
    void getCacheStats(uint32_t& hits, uint32_t& misses, uint32_t& entries);
    
private:
    // Translation cache
    std::unordered_map<uint32_t, uint32_t> translations;
    
    // Cache statistics
    uint32_t cache_hits;
    uint32_t cache_misses;
    uint32_t cache_entries;
};

// Global memory access translation cache
extern MemoryAccessTranslationCache g_memory_translation_cache;
