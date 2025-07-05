#ifndef ARM_TRANSLATOR_H
#define ARM_TRANSLATOR_H

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

// Forward declarations
struct CPUContext;
class ICPUBackend;
class JITManager;
class DynarmicJITManager;

// Translation block status
enum class TranslationStatus {
    NOT_TRANSLATED,    // Block has not been translated yet
    TRANSLATED,        // Block has been translated and is ready for execution
    FAILED,            // Translation failed
    INVALIDATED        // Translation was invalidated and needs to be retranslated
};

// Translation block information
struct TranslationBlock {
    uint32_t vita_addr;            // PS Vita address of the block
    uint32_t switch_addr;          // Nintendo Switch address of the translated code
    uint32_t size;                 // Size of the original block in bytes
    uint32_t translated_size;      // Size of the translated block in bytes
    TranslationStatus status;      // Current status of the block
    uint64_t execution_count;      // Number of times this block has been executed
    uint64_t total_cycles;         // Total cycles spent in this block
    bool is_thumb;                 // Whether this is a Thumb mode block
    void* native_code;             // Pointer to the native code (if JIT)
};

// ARM-to-ARM translator interface
class ARMTranslator {
public:
    virtual ~ARMTranslator() = default;

    // Initialize the translator
    virtual bool initialize(ICPUBackend* cpu_backend) = 0;
    
    // Translate a block of ARM code
    virtual TranslationBlock* translateBlock(uint32_t addr, bool is_thumb) = 0;
    
    // Execute a translated block
    virtual int executeBlock(TranslationBlock* block, CPUContext& context) = 0;
    
    // Invalidate a translated block
    virtual void invalidateBlock(uint32_t addr) = 0;
    
    // Invalidate all blocks in a range
    virtual void invalidateRange(uint32_t start_addr, uint32_t end_addr) = 0;
    
    // Get statistics
    virtual uint64_t getTotalExecutedBlocks() const = 0;
    virtual uint64_t getTotalTranslatedBlocks() const = 0;
    virtual uint64_t getTotalTranslationTime() const = 0;
    virtual uint64_t getTotalExecutionTime() const = 0;
    
    // Flush the translation cache
    virtual void flushCache() = 0;
};

// JIT compilation manager
class JITManager {
public:
    virtual ~JITManager() = default;
    
    // Initialize the JIT compiler
    virtual bool initialize() = 0;
    
    // Compile a block of ARM code to native code
    virtual void* compileBlock(const uint8_t* code, size_t size, uint32_t addr, bool is_thumb) = 0;
    
    // Free compiled code
    virtual void freeCompiledCode(void* native_code) = 0;
    
    // Flush the JIT cache
    virtual void flushCache() = 0;
};

// ARM-to-ARM direct translator (stub implementation)
class DirectARMTranslator : public ARMTranslator {
public:
    DirectARMTranslator();
    ~DirectARMTranslator();
    
    bool initialize(ICPUBackend* cpu_backend) override;
    TranslationBlock* translateBlock(uint32_t addr, bool is_thumb) override;
    int executeBlock(TranslationBlock* block, CPUContext& context) override;
    void invalidateBlock(uint32_t addr) override;
    void invalidateRange(uint32_t start_addr, uint32_t end_addr) override;
    uint64_t getTotalExecutedBlocks() const override;
    uint64_t getTotalTranslatedBlocks() const override;
    uint64_t getTotalTranslationTime() const override;
    uint64_t getTotalExecutionTime() const override;
    void flushCache() override;
    
private:
    ICPUBackend* cpu_backend;
    std::unordered_map<uint32_t, std::unique_ptr<TranslationBlock>> blocks;
    uint64_t total_executed_blocks;
    uint64_t total_translated_blocks;
    uint64_t total_translation_time;
    uint64_t total_execution_time;
    std::mutex mutex;
    
    // Helper methods
    TranslationBlock* findBlock(uint32_t addr);
    bool readMemory(uint32_t addr, void* buffer, size_t size);
};

// Factory function to create an appropriate translator based on configuration
std::unique_ptr<ARMTranslator> createARMTranslator(bool use_jit);

class DynarmicJITManager;

#endif // ARM_TRANSLATOR_H
