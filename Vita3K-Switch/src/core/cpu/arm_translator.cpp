#include "arm_translator.h"
#include "cpu_backend.h"
#include "dynarmic_jit.h"
#include <cstdio>
#include <chrono>
#include <algorithm>
#include <memory>
#ifdef USE_DYNARMIC
#include <dynarmic/A32/user_callbacks.h>
#include <dynarmic/A32/jit.h>
#endif

// DirectARMTranslator implementation
DirectARMTranslator::DirectARMTranslator()
    : cpu_backend(nullptr)
    , total_executed_blocks(0)
    , total_translated_blocks(0)
    , total_translation_time(0)
    , total_execution_time(0) {
}

DirectARMTranslator::~DirectARMTranslator() {
    flushCache();
}

bool DirectARMTranslator::initialize(ICPUBackend* cpu_backend) {
    if (!cpu_backend) {
        printf("[ARMTranslator] Error: Invalid CPU backend\n");
        return false;
    }
    
    this->cpu_backend = cpu_backend;
    printf("[ARMTranslator] Initialized ARM-to-ARM translator\n");
    return true;
}

TranslationBlock* DirectARMTranslator::translateBlock(uint32_t addr, bool is_thumb) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Check if block already exists
    TranslationBlock* block = findBlock(addr);
    if (block) {
        if (block->status == TranslationStatus::TRANSLATED) {
            return block;
        } else if (block->status == TranslationStatus::INVALIDATED) {
            // Block needs to be retranslated
            blocks.erase(addr);
            block = nullptr;
        }
    }
    
    // Start translation timer
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create a new block
    auto new_block = std::make_unique<TranslationBlock>();
    new_block->vita_addr = addr;
    new_block->switch_addr = addr;  // In a direct translator, addresses are the same
    new_block->is_thumb = is_thumb;
    new_block->execution_count = 0;
    new_block->total_cycles = 0;
    new_block->native_code = nullptr;
    
    // In a real implementation, we would analyze the code to find the block size
    // For now, use a fixed size
    const uint32_t MAX_BLOCK_SIZE = 256;  // Maximum block size in bytes
    new_block->size = MAX_BLOCK_SIZE;
    new_block->translated_size = MAX_BLOCK_SIZE;
    
    // In a real implementation, we would translate the code here
    // For now, just mark it as translated
    new_block->status = TranslationStatus::TRANSLATED;
    
    // End translation timer
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    total_translation_time += duration;
    
    total_translated_blocks++;
    
    // Store the block
    TranslationBlock* result = new_block.get();
    blocks[addr] = std::move(new_block);
    
    printf("[ARMTranslator] Translated block at 0x%08X (thumb=%d)\n", addr, is_thumb);
    return result;
}

int DirectARMTranslator::executeBlock(TranslationBlock* block, CPUContext& context) {
    if (!block) {
        printf("[ARMTranslator] Error: Invalid block\n");
        return -1;
    }
    
    if (block->status != TranslationStatus::TRANSLATED) {
        printf("[ARMTranslator] Error: Block not translated\n");
        return -1;
    }
    
    // Start execution timer
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // In a real implementation, we would execute the translated code here
    // For now, just increment the block's execution count and return success
    block->execution_count++;
    total_executed_blocks++;
    
    // End execution timer
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    block->total_cycles += duration;
    total_execution_time += duration;
    
    return 0;
}

void DirectARMTranslator::invalidateBlock(uint32_t addr) {
    std::lock_guard<std::mutex> lock(mutex);
    
    TranslationBlock* block = findBlock(addr);
    if (block) {
        block->status = TranslationStatus::INVALIDATED;
        printf("[ARMTranslator] Invalidated block at 0x%08X\n", addr);
    }
}

void DirectARMTranslator::invalidateRange(uint32_t start_addr, uint32_t end_addr) {
    std::lock_guard<std::mutex> lock(mutex);
    
    for (auto& pair : blocks) {
        TranslationBlock* block = pair.second.get();
        if (block->vita_addr >= start_addr && block->vita_addr < end_addr) {
            block->status = TranslationStatus::INVALIDATED;
        }
    }
    
    printf("[ARMTranslator] Invalidated blocks in range 0x%08X - 0x%08X\n", start_addr, end_addr);
}

uint64_t DirectARMTranslator::getTotalExecutedBlocks() const {
    return total_executed_blocks;
}

uint64_t DirectARMTranslator::getTotalTranslatedBlocks() const {
    return total_translated_blocks;
}

uint64_t DirectARMTranslator::getTotalTranslationTime() const {
    return total_translation_time;
}

uint64_t DirectARMTranslator::getTotalExecutionTime() const {
    return total_execution_time;
}

void DirectARMTranslator::flushCache() {
    std::lock_guard<std::mutex> lock(mutex);
    
    blocks.clear();
    total_translated_blocks = 0;
    
    printf("[ARMTranslator] Flushed translation cache\n");
}

TranslationBlock* DirectARMTranslator::findBlock(uint32_t addr) {
    auto it = blocks.find(addr);
    if (it != blocks.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool DirectARMTranslator::readMemory(uint32_t addr, void* buffer, size_t size) {
    if (!cpu_backend) {
        printf("[DirectARMTranslator] Error: CPU backend not initialized\n");
        return false;
    }
    
    // Use the CPU backend to read memory
    return cpu_backend->readMemory(addr, buffer, size);
}

// Factory function implementation
std::unique_ptr<ARMTranslator> createARMTranslator(bool use_jit) {
#ifdef USE_DYNARMIC
    if (use_jit) {
        printf("[ARMTranslator] Creating Dynarmic JIT-enabled ARM translator\n");
        return std::make_unique<DynarmicARMTranslator>();
    } else {
        printf("[ARMTranslator] Creating direct ARM translator\n");
        return std::make_unique<DirectARMTranslator>();
    }
#else
    if (use_jit) {
        printf("[ARMTranslator] Dynarmic not available, falling back to direct ARM translator\n");
    } else {
        printf("[ARMTranslator] Creating direct ARM translator\n");
    }
    return std::make_unique<DirectARMTranslator>();
#endif
}
