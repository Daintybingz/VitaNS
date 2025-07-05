#ifndef DYNARMIC_JIT_H
#define DYNARMIC_JIT_H

#pragma once

#include "core/cpu/arm_translator.h"
#include "core/cpu/dynarmic_memory_interface.h"

// Check if Dynarmic is available
#if defined(HAVE_DYNARMIC) || defined(USE_DYNARMIC)
#define USE_DYNARMIC
#include <dynarmic/interface/A32/a32.h>
#endif

#include <memory>
#include <unordered_map>
#include <vector>

// Forward declarations for Dynarmic
namespace Dynarmic {
namespace A32 {
    class Jit;
    struct UserCallbacks;
}
}

// Dynarmic JIT implementation
class DynarmicJITManager : public JITManager {
public:
    DynarmicJITManager(ICPUBackend* cpu_backend);
    ~DynarmicJITManager();
    bool initialize() override;
    void flushCache() override;
    void* compileBlock(const uint8_t* code, size_t size, uint32_t addr, bool is_thumb) override;
    void freeCompiledCode(void* native_code) override;
    DynarmicMemoryInterface* getMemoryInterface() { return memory_interface.get(); }
    void setCPSR(uint32_t cpsr);
    uint32_t getCPSR();
    void runJit();
    void step();
    void setRegister(size_t index, uint32_t value);
    uint32_t getRegister(size_t index);
    void saveContext(CPUContext& context);
    void loadContext(const CPUContext& context);
    void createJit();
    void setPC(uint32_t pc);
    uint32_t getPC();

private:
    struct CompiledBlock {
        uint32_t addr;
        uint32_t size;
        bool is_thumb;
    };
    std::unique_ptr<DynarmicMemoryInterface> memory_interface;
    ICPUBackend* cpu_backend;
    bool initialized;
    std::unordered_map<void*, CompiledBlock> compiled_blocks;
#ifdef USE_DYNARMIC
    std::unique_ptr<Dynarmic::A32::Jit> jit;
    std::unique_ptr<Dynarmic::A32::UserCallbacks> callbacks;
#endif
};

// Dynarmic-based ARM translator
class DynarmicARMTranslator : public ARMTranslator {
public:
    DynarmicARMTranslator(ICPUBackend* cpu_backend);
    ~DynarmicARMTranslator();
    
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
    std::unique_ptr<DynarmicJITManager> jit_manager;
    std::unordered_map<uint32_t, std::unique_ptr<TranslationBlock>> blocks;
    uint64_t total_executed_blocks;
    uint64_t total_translated_blocks;
    uint64_t total_translation_time;
    uint64_t total_execution_time;
    std::mutex mutex;
    
    // Helper methods
    TranslationBlock* findBlock(uint32_t addr);
    bool analyzeBlockBoundaries(uint32_t addr, bool is_thumb, uint32_t& size);
};

// Factory function to create a Dynarmic-based translator
std::unique_ptr<ARMTranslator> createDynarmicTranslator(ICPUBackend* cpu_backend);

#endif // DYNARMIC_JIT_H
