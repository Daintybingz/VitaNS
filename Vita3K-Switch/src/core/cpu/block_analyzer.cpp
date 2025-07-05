#include "block_analyzer.h"
#include "switch_cpu_backend.h"
#include <cstdio>
#include <algorithm>
#include <queue>

// CPUBackendMemoryAdapter implementation
CPUBackendMemoryAdapter::CPUBackendMemoryAdapter(ICPUBackend* cpu_backend)
    : cpu_backend(cpu_backend) {
}

CPUBackendMemoryAdapter::~CPUBackendMemoryAdapter() {
}

bool CPUBackendMemoryAdapter::readBytes(uint32_t address, void* buffer, size_t size) {
    // In a real implementation, this would use the CPU backend to read memory
    // For now, just return false
    return false;
}

bool CPUBackendMemoryAdapter::isExecutable(uint32_t address) {
    // In a real implementation, this would check memory permissions
    return true;
}

// BlockAnalyzer implementation
BlockAnalyzer::BlockAnalyzer(std::shared_ptr<BlockAnalyzerMemory> memory)
    : memory(memory) {
}

BlockAnalyzer::~BlockAnalyzer() {
}

std::unique_ptr<BasicBlock> BlockAnalyzer::analyzeBlock(uint32_t address, bool is_thumb) {
    if (!memory) {
        printf("[BlockAnalyzer] Error: No memory interface\n");
        return nullptr;
    }
    
    // Create a new basic block
    auto block = std::make_unique<BasicBlock>();
    block->start_address = address;
    block->is_thumb = is_thumb;
    
    // Analyze instructions until we find the end of the block
    uint32_t current_address = address;
    bool end_of_block = false;
    
    while (!end_of_block) {
        // Decode the instruction at the current address
        ARMInstruction inst = decodeInstruction(current_address, is_thumb);
        
        // Add the instruction to the block
        block->instructions.push_back(inst);
        
        // Check if this instruction ends the block
        end_of_block = isBlockEnd(inst);
        
        // Move to the next instruction
        current_address += inst.size;
        
        // Safety check: limit block size
        if (block->instructions.size() >= 1000) {
            printf("[BlockAnalyzer] Warning: Block too large, truncating at 0x%08X\n", current_address);
            end_of_block = true;
        }
    }
    
    // Set block end address and size
    block->end_address = current_address - 1;
    block->size = block->end_address - block->start_address + 1;
    
    // Set block exit type
    const ARMInstruction& last_inst = block->instructions.back();
    block->exit_type = last_inst.branch;
    
    // Add successors
    if (last_inst.isDirectBranch()) {
        block->successors.push_back(last_inst.target);
    }
    
    if (last_inst.isConditionalBranch()) {
        // Add fall-through path for conditional branches
        block->successors.push_back(current_address);
    }
    
    return block;
}

std::vector<std::unique_ptr<BasicBlock>> BlockAnalyzer::analyzeFunction(uint32_t address, bool is_thumb) {
    std::vector<std::unique_ptr<BasicBlock>> blocks;
    std::unordered_set<uint32_t> visited;
    std::queue<std::pair<uint32_t, bool>> work_list;
    
    // Start with the entry point
    work_list.push({address, is_thumb});
    
    while (!work_list.empty()) {
        // Get the next block to analyze
        auto [block_addr, block_thumb] = work_list.front();
        work_list.pop();
        
        // Skip if already visited
        if (visited.find(block_addr) != visited.end()) {
            continue;
        }
        
        // Mark as visited
        visited.insert(block_addr);
        
        // Analyze the block
        auto block = analyzeBlock(block_addr, block_thumb);
        if (!block) {
            continue;
        }
        
        // Add successors to work list
        for (uint32_t succ : block->successors) {
            if (visited.find(succ) == visited.end()) {
                work_list.push({succ, block->is_thumb});
            }
        }
        
        // Add the block to the result
        blocks.push_back(std::move(block));
        
        // Safety check: limit number of blocks
        if (blocks.size() >= 1000) {
            printf("[BlockAnalyzer] Warning: Function too large, truncating at %zu blocks\n", blocks.size());
            break;
        }
    }
    
    return blocks;
}

ARMInstruction BlockAnalyzer::decodeInstruction(uint32_t address, bool is_thumb) {
    ARMInstruction inst;
    inst.address = address;
    inst.is_thumb = is_thumb;
    inst.size = is_thumb ? 2 : 4;
    inst.branch = ARMBranchType::NONE;
    inst.target = 0;
    
    // Read the instruction encoding
    if (!readInstruction(address, is_thumb, inst.encoding)) {
        printf("[BlockAnalyzer] Error: Failed to read instruction at 0x%08X\n", address);
        return inst;
    }
    
    // Check for Thumb2 (32-bit Thumb instruction)
    if (is_thumb && (inst.encoding & 0xF800) >= 0xE800) {
        // This is a 32-bit Thumb instruction
        inst.size = 4;
        
        // Read the second half of the instruction
        uint32_t second_half;
        if (!readInstruction(address + 2, is_thumb, second_half)) {
            printf("[BlockAnalyzer] Error: Failed to read second half of Thumb2 instruction at 0x%08X\n", address + 2);
            return inst;
        }
        
        // Combine the two halves
        inst.encoding = (inst.encoding << 16) | second_half;
    }
    
    // Identify branch type
    inst.branch = identifyBranchType(inst.encoding, is_thumb);
    
    // Calculate branch target for direct branches
    if (inst.isDirectBranch() || inst.isConditionalBranch()) {
        inst.target = calculateBranchTarget(address, inst.encoding, is_thumb);
    }
    
    return inst;
}

bool BlockAnalyzer::readInstruction(uint32_t address, bool is_thumb, uint32_t& encoding) {
    // Read 2 or 4 bytes depending on mode
    size_t size = is_thumb ? 2 : 4;
    uint8_t buffer[4] = {0};
    
    if (!memory->readBytes(address, buffer, size)) {
        return false;
    }
    
    // Combine bytes into instruction encoding
    encoding = 0;
    for (size_t i = 0; i < size; i++) {
        encoding |= static_cast<uint32_t>(buffer[i]) << (8 * i);
    }
    
    return true;
}

ARMBranchType BlockAnalyzer::identifyBranchType(uint32_t encoding, bool is_thumb) {
    if (is_thumb) {
        // Thumb mode
        ARMInstruction inst = decodeInstruction(0, is_thumb); // Decode instruction
        if (inst.size == 2) {
            // 16-bit Thumb instructions
            uint16_t op = static_cast<uint16_t>(encoding);
            
            // Conditional branch (B<cond>)
            if ((op & 0xF000) == 0xD000) {
                return ARMBranchType::CONDITIONAL;
            }
            
            // Unconditional branch (B)
            if ((op & 0xF800) == 0xE000) {
                return ARMBranchType::DIRECT;
            }
            
            // BX/BLX register
            if ((op & 0xFF80) == 0x4780) {
                // BX LR is a return
                if ((op & 0x0078) == 0x0070) {
                    return ARMBranchType::RETURN;
                }
                return ARMBranchType::INDIRECT;
            }
            
            // POP {..., PC}
            if ((op & 0xFF00) == 0xBD00 && (op & 0x0100)) {
                return ARMBranchType::RETURN;
            }
        } else {
            // 32-bit Thumb instructions
            uint32_t op = encoding;
            
            // Branches and miscellaneous control
            if ((op & 0xF800D000) == 0xF0008000) {
                // B.W (unconditional)
                return ARMBranchType::DIRECT;
            }
            
            if ((op & 0xF800D000) == 0xF0009000) {
                // B.W (conditional)
                return ARMBranchType::CONDITIONAL;
            }
            
            // BL/BLX
            if ((op & 0xF800D000) == 0xF000D000) {
                return ARMBranchType::DIRECT;
            }
            
            // Table branch
            if ((op & 0xFFF0D000) == 0xE890D000) {
                return ARMBranchType::TABLE;
            }
        }
    } else {
        // ARM mode
        uint32_t op = encoding;
        
        // Branch and Branch with Link (B, BL)
        if ((op & 0x0E000000) == 0x0A000000) {
            return ARMBranchType::DIRECT;
        }
        
        // Branch and Exchange (BX)
        if ((op & 0x0FFFFFF0) == 0x012FFF10) {
            // BX LR is a return
            if ((op & 0x0000000F) == 0x0000000E) {
                return ARMBranchType::RETURN;
            }
            return ARMBranchType::INDIRECT;
        }
        
        // Branch and Link with Exchange (BLX)
        if ((op & 0x0FFFFFF0) == 0x012FFF30) {
            return ARMBranchType::INDIRECT;
        }
        
        // LDM with PC in register list
        if ((op & 0x0E108000) == 0x08108000) {
            return ARMBranchType::RETURN;
        }
    }
    
    return ARMBranchType::NONE;
}

uint32_t BlockAnalyzer::calculateBranchTarget(uint32_t address, uint32_t encoding, bool is_thumb) {
    if (is_thumb) {
        // Thumb mode
        ARMInstruction inst = decodeInstruction(address, is_thumb); // Decode instruction
        if (inst.size == 2) {
            // 16-bit Thumb instructions
            uint16_t op = static_cast<uint16_t>(encoding);
            
            // Conditional branch (B<cond>)
            if ((op & 0xF000) == 0xD000) {
                // 8-bit signed immediate, shifted left by 1
                int32_t offset = ((op & 0x00FF) << 1);
                if (offset & 0x0100) {
                    offset |= 0xFFFFFE00;  // Sign extend
                }
                return address + 4 + offset;
            }
            
            // Unconditional branch (B)
            if ((op & 0xF800) == 0xE000) {
                // 11-bit signed immediate, shifted left by 1
                int32_t offset = ((op & 0x07FF) << 1);
                if (offset & 0x0800) {
                    offset |= 0xFFFFF000;  // Sign extend
                }
                return address + 4 + offset;
            }
        } else {
            // 32-bit Thumb instructions
            uint32_t op = encoding;
            
            // B.W (unconditional or conditional)
            if ((op & 0xF800D000) == 0xF0008000 || (op & 0xF800D000) == 0xF0009000) {
                // Complex encoding with S, J1, J2, imm10, imm11 fields
                uint32_t S = (op >> 26) & 1;
                uint32_t J1 = (op >> 13) & 1;
                uint32_t J2 = (op >> 11) & 1;
                uint32_t imm10 = (op >> 16) & 0x3FF;
                uint32_t imm11 = op & 0x7FF;
                
                uint32_t I1 = ~(J1 ^ S) & 1;
                uint32_t I2 = ~(J2 ^ S) & 1;
                
                int32_t offset = (S << 24) | (I1 << 23) | (I2 << 22) | (imm10 << 12) | (imm11 << 1);
                if (S) {
                    offset |= 0xFE000000;  // Sign extend
                }
                
                return address + 4 + offset;
            }
            
            // BL/BLX
            if ((op & 0xF800D000) == 0xF000D000) {
                // Similar encoding to B.W
                uint32_t S = (op >> 26) & 1;
                uint32_t J1 = (op >> 13) & 1;
                uint32_t J2 = (op >> 11) & 1;
                uint32_t imm10 = (op >> 16) & 0x3FF;
                uint32_t imm11 = op & 0x7FF;
                
                uint32_t I1 = ~(J1 ^ S) & 1;
                uint32_t I2 = ~(J2 ^ S) & 1;
                
                int32_t offset = (S << 24) | (I1 << 23) | (I2 << 22) | (imm10 << 12) | (imm11 << 1);
                if (S) {
                    offset |= 0xFE000000;  // Sign extend
                }
                
                return address + 4 + offset;
            }
        }
    } else {
        // ARM mode
        uint32_t op = encoding;
        
        // Branch and Branch with Link (B, BL)
        if ((op & 0x0E000000) == 0x0A000000) {
            // 24-bit signed immediate, shifted left by 2
            int32_t offset = ((op & 0x00FFFFFF) << 2);
            if (offset & 0x02000000) {
                offset |= 0xFC000000;  // Sign extend
            }
            return address + 8 + offset;
        }
    }
    
    return 0;
}

bool BlockAnalyzer::isBlockEnd(const ARMInstruction& inst) {
    // Any branch instruction ends a block
    if (inst.isBranch()) {
        return true;
    }
    
    // TODO: Add more conditions for block end
    
    return false;
}
