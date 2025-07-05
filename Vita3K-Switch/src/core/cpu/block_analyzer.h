#pragma once

#include <cstdint>
#include <vector>
#include <unordered_set>
#include <memory>

// ARM instruction types that affect control flow
enum class ARMBranchType {
    NONE,           // Not a branch
    DIRECT,         // Direct branch (B, BL)
    INDIRECT,       // Indirect branch (BX, BLX reg)
    CONDITIONAL,    // Conditional branch (B<cond>)
    RETURN,         // Return (BX LR, POP {..., PC})
    TABLE,          // Table branch (TBB, TBH)
    UNKNOWN         // Unknown or complex branch
};

// ARM instruction information
struct ARMInstruction {
    uint32_t address;        // Address of the instruction
    uint32_t encoding;       // Raw instruction encoding
    uint32_t size;           // Size in bytes (2 for Thumb, 4 for ARM)
    ARMBranchType branch;    // Branch type
    uint32_t target;         // Branch target address (if direct)
    bool is_thumb;           // Whether this is a Thumb instruction
    
    // Helper methods
    bool isBranch() const { return branch != ARMBranchType::NONE; }
    bool isDirectBranch() const { return branch == ARMBranchType::DIRECT; }
    bool isIndirectBranch() const { return branch == ARMBranchType::INDIRECT; }
    bool isConditionalBranch() const { return branch == ARMBranchType::CONDITIONAL; }
    bool isReturn() const { return branch == ARMBranchType::RETURN; }
    bool isTableBranch() const { return branch == ARMBranchType::TABLE; }
};

// Basic block information
struct BasicBlock {
    uint32_t start_address;              // Start address of the block
    uint32_t end_address;                // End address of the block
    uint32_t size;                       // Size in bytes
    std::vector<ARMInstruction> instructions; // Instructions in the block
    ARMBranchType exit_type;             // How the block exits
    std::vector<uint32_t> successors;    // Addresses of successor blocks
    bool is_thumb;                       // Whether this is a Thumb block
    
    // Helper methods
    bool isTerminal() const {
        return exit_type == ARMBranchType::RETURN || 
               exit_type == ARMBranchType::INDIRECT ||
               exit_type == ARMBranchType::UNKNOWN;
    }
};

// Memory interface for block analyzer
class BlockAnalyzerMemory {
public:
    virtual ~BlockAnalyzerMemory() = default;
    
    // Read memory
    virtual bool readBytes(uint32_t address, void* buffer, size_t size) = 0;
    
    // Check if address is executable
    virtual bool isExecutable(uint32_t address) = 0;
};

// Block analyzer
class BlockAnalyzer {
public:
    BlockAnalyzer(std::shared_ptr<BlockAnalyzerMemory> memory);
    ~BlockAnalyzer();
    
    // Analyze a basic block starting at the given address
    std::unique_ptr<BasicBlock> analyzeBlock(uint32_t address, bool is_thumb);
    
    // Analyze a function starting at the given address
    std::vector<std::unique_ptr<BasicBlock>> analyzeFunction(uint32_t address, bool is_thumb);
    
    // Decode a single instruction
    ARMInstruction decodeInstruction(uint32_t address, bool is_thumb);
    
private:
    std::shared_ptr<BlockAnalyzerMemory> memory;
    
    // Helper methods
    bool readInstruction(uint32_t address, bool is_thumb, uint32_t& encoding);
    ARMBranchType identifyBranchType(uint32_t encoding, bool is_thumb);
    uint32_t calculateBranchTarget(uint32_t address, uint32_t encoding, bool is_thumb);
    bool isBlockEnd(const ARMInstruction& inst);
};

// CPU backend memory adapter
class CPUBackendMemoryAdapter : public BlockAnalyzerMemory {
public:
    CPUBackendMemoryAdapter(class ICPUBackend* cpu_backend);
    ~CPUBackendMemoryAdapter();
    
    bool readBytes(uint32_t address, void* buffer, size_t size) override;
    bool isExecutable(uint32_t address) override;
    
private:
    class ICPUBackend* cpu_backend;
};
