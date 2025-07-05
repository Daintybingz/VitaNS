#include "register_allocator.h"
#include <cstdio>
#include <algorithm>
#include <queue>

// RegisterAllocator implementation
RegisterAllocator::RegisterAllocator()
    : total_reads(0), total_writes(0), spills(0), reloads(0) {
    // Initialize register usage
    register_usage.resize(NUM_VITA_REGS);
    
    // Initialize interference graph
    interference_graph.resize(NUM_VITA_REGS);
}

RegisterAllocator::~RegisterAllocator() {
}

void RegisterAllocator::analyzeRegisterUsage(const BasicBlock& block) {
    // Reset register usage
    register_usage.clear();
    register_usage.resize(NUM_VITA_REGS);
    
    // Reset instruction allocations
    instruction_allocations.clear();
    instruction_allocations.resize(block.instructions.size());
    
    // Reset statistics
    total_reads = 0;
    total_writes = 0;
    spills = 0;
    reloads = 0;
    
    // Analyze register usage for each instruction
    for (size_t i = 0; i < block.instructions.size(); i++) {
        analyzeInstructionRegisterUsage(block.instructions[i], i);
    }
    
    // Build interference graph
    buildInterferenceGraph();
    
    // Allocate registers using linear scan algorithm
    allocateRegistersLinearScan();
}

std::vector<RegisterAllocation> RegisterAllocator::allocateRegisters(const BasicBlock& block) {
    // Analyze register usage first
    analyzeRegisterUsage(block);
    
    // Create register allocations
    std::vector<RegisterAllocation> allocations(NUM_VITA_REGS);
    
    // Assign registers based on the linear scan allocation
    for (uint8_t i = 0; i < NUM_VITA_REGS; i++) {
        if (register_usage[i].read || register_usage[i].written) {
            // Find the allocation for this register
            for (const auto& inst_alloc : instruction_allocations) {
                for (const auto& alloc : inst_alloc) {
                    if (alloc.vita_reg == i) {
                        allocations[i] = alloc;
                        break;
                    }
                }
            }
        }
    }
    
    return allocations;
}

std::vector<RegisterAllocation> RegisterAllocator::getInstructionAllocation(uint32_t inst_index) {
    if (inst_index < instruction_allocations.size()) {
        return instruction_allocations[inst_index];
    }
    return {};
}

void RegisterAllocator::getRegisterUsageStats(uint32_t& total_reads, uint32_t& total_writes, 
                                           uint32_t& spills, uint32_t& reloads) {
    total_reads = this->total_reads;
    total_writes = this->total_writes;
    spills = this->spills;
    reloads = this->reloads;
}

void RegisterAllocator::analyzeInstructionRegisterUsage(const ARMInstruction& inst, uint32_t inst_index) {
    // This is a stub implementation. In a real implementation, we would analyze
    // the instruction to determine which registers are read and written.
    
    // For now, let's simulate some register usage based on instruction type
    if (inst.is_thumb) {
        // Thumb mode
        if (inst.size == 2) {
            // 16-bit Thumb instruction
            uint16_t op = static_cast<uint16_t>(inst.encoding);
            
            // Example: ADD/SUB/MOV/CMP immediate (format 1)
            if ((op & 0xE000) == 0x0000) {
                uint8_t rd = op & 0x7;
                uint8_t rn = (op >> 3) & 0x7;
                
                // Read rn, write rd
                if (!register_usage[rn].read) {
                    register_usage[rn].read = true;
                    register_usage[rn].first_read = true;
                    register_usage[rn].first_use = inst_index;
                }
                register_usage[rn].last_use = inst_index;
                total_reads++;
                
                if (!register_usage[rd].written) {
                    register_usage[rd].written = true;
                    if (!register_usage[rd].read) {
                        register_usage[rd].first_use = inst_index;
                    }
                }
                register_usage[rd].last_use = inst_index;
                register_usage[rd].last_write = true;
                total_writes++;
            }
            
            // Example: Branch
            else if ((op & 0xF000) == 0xD000 || (op & 0xF800) == 0xE000) {
                // Conditional branch or unconditional branch
                // No register usage (PC is implicit)
            }
            
            // Example: BX/BLX
            else if ((op & 0xFF80) == 0x4780) {
                uint8_t rm = (op >> 3) & 0xF;
                
                // Read rm
                if (!register_usage[rm].read) {
                    register_usage[rm].read = true;
                    register_usage[rm].first_read = true;
                    register_usage[rm].first_use = inst_index;
                }
                register_usage[rm].last_use = inst_index;
                total_reads++;
                
                // Write LR for BLX
                if ((op & 0x0080) == 0x0080) {
                    if (!register_usage[LR_REG].written) {
                        register_usage[LR_REG].written = true;
                        if (!register_usage[LR_REG].read) {
                            register_usage[LR_REG].first_use = inst_index;
                        }
                    }
                    register_usage[LR_REG].last_use = inst_index;
                    register_usage[LR_REG].last_write = true;
                    total_writes++;
                }
            }
            
            // Add more Thumb instruction patterns as needed
        } else {
            // 32-bit Thumb instruction
            uint32_t op = inst.encoding;
            
            // Example: Data processing (register)
            if ((op & 0xEF000000) == 0xEA000000) {
                uint8_t rn = (op >> 16) & 0xF;
                uint8_t rd = (op >> 8) & 0xF;
                uint8_t rm = op & 0xF;
                
                // Read rn and rm, write rd
                if (!register_usage[rn].read) {
                    register_usage[rn].read = true;
                    register_usage[rn].first_read = true;
                    register_usage[rn].first_use = inst_index;
                }
                register_usage[rn].last_use = inst_index;
                total_reads++;
                
                if (!register_usage[rm].read) {
                    register_usage[rm].read = true;
                    register_usage[rm].first_read = true;
                    register_usage[rm].first_use = inst_index;
                }
                register_usage[rm].last_use = inst_index;
                total_reads++;
                
                if (!register_usage[rd].written) {
                    register_usage[rd].written = true;
                    if (!register_usage[rd].read) {
                        register_usage[rd].first_use = inst_index;
                    }
                }
                register_usage[rd].last_use = inst_index;
                register_usage[rd].last_write = true;
                total_writes++;
            }
            
            // Add more Thumb2 instruction patterns as needed
        }
    } else {
        // ARM mode
        uint32_t op = inst.encoding;
        
        // Example: Data processing (register)
        if ((op & 0x0C000000) == 0x00000000) {
            uint8_t rn = (op >> 16) & 0xF;
            uint8_t rd = (op >> 12) & 0xF;
            uint8_t rm = op & 0xF;
            
            // Read rn and rm, write rd
            if (!register_usage[rn].read) {
                register_usage[rn].read = true;
                register_usage[rn].first_read = true;
                register_usage[rn].first_use = inst_index;
            }
            register_usage[rn].last_use = inst_index;
            total_reads++;
            
            if (!register_usage[rm].read) {
                register_usage[rm].read = true;
                register_usage[rm].first_read = true;
                register_usage[rm].first_use = inst_index;
            }
            register_usage[rm].last_use = inst_index;
            total_reads++;
            
            if (!register_usage[rd].written) {
                register_usage[rd].written = true;
                if (!register_usage[rd].read) {
                    register_usage[rd].first_use = inst_index;
                }
            }
            register_usage[rd].last_use = inst_index;
            register_usage[rd].last_write = true;
            total_writes++;
        }
        
        // Example: Branch
        else if ((op & 0x0E000000) == 0x0A000000) {
            // No register usage (PC is implicit)
        }
        
        // Example: BX/BLX
        else if ((op & 0x0FFFFFF0) == 0x012FFF10 || (op & 0x0FFFFFF0) == 0x012FFF30) {
            uint8_t rm = op & 0xF;
            
            // Read rm
            if (!register_usage[rm].read) {
                register_usage[rm].read = true;
                register_usage[rm].first_read = true;
                register_usage[rm].first_use = inst_index;
            }
            register_usage[rm].last_use = inst_index;
            total_reads++;
            
            // Write LR for BLX
            if ((op & 0x0FFFFFF0) == 0x012FFF30) {
                if (!register_usage[LR_REG].written) {
                    register_usage[LR_REG].written = true;
                    if (!register_usage[LR_REG].read) {
                        register_usage[LR_REG].first_use = inst_index;
                    }
                }
                register_usage[LR_REG].last_use = inst_index;
                register_usage[LR_REG].last_write = true;
                total_writes++;
            }
        }
        
        // Add more ARM instruction patterns as needed
    }
}

void RegisterAllocator::buildInterferenceGraph() {
    // Reset interference graph
    interference_graph.clear();
    interference_graph.resize(NUM_VITA_REGS);
    
    // Build live ranges
    live_ranges.clear();
    for (uint8_t i = 0; i < NUM_VITA_REGS; i++) {
        if (register_usage[i].read || register_usage[i].written) {
            live_ranges.emplace_back(i, register_usage[i].first_use, register_usage[i].last_use);
        }
    }
    
    // Build interference graph
    for (size_t i = 0; i < live_ranges.size(); i++) {
        for (size_t j = i + 1; j < live_ranges.size(); j++) {
            // Check if live ranges overlap
            if (live_ranges[i].start <= live_ranges[j].end && live_ranges[j].start <= live_ranges[i].end) {
                // Add interference edge
                interference_graph[live_ranges[i].reg].insert(live_ranges[j].reg);
                interference_graph[live_ranges[j].reg].insert(live_ranges[i].reg);
            }
        }
    }
}

void RegisterAllocator::allocateRegistersLinearScan() {
    // Sort live ranges by start position
    std::sort(live_ranges.begin(), live_ranges.end());
    
    // Set of active live ranges
    std::set<LiveRange*, decltype([](const LiveRange* a, const LiveRange* b) {
        return a->end < b->end;
    })> active;
    
    // Set of available physical registers
    std::set<uint8_t> available;
    for (uint8_t i = 0; i < NUM_SWITCH_REGS; i++) {
        // Reserve SP, LR, and PC
        if (i != SP_REG && i != LR_REG && i != PC_REG) {
            available.insert(i);
        }
    }
    
    // Map of virtual to physical registers
    std::unordered_map<uint8_t, uint8_t> reg_map;
    
    // Map of spilled registers to spill slots
    std::unordered_map<uint8_t, uint32_t> spill_map;
    uint32_t next_spill_slot = 0;
    
    // Linear scan algorithm
    for (auto& range : live_ranges) {
        // Expire old ranges
        for (auto it = active.begin(); it != active.end(); ) {
            if ((*it)->end < range.start) {
                // Return the physical register to the pool
                available.insert(reg_map[(*it)->reg]);
                active.erase(it++);
            } else {
                ++it;
            }
        }
        
        // Try to allocate a physical register
        if (!available.empty()) {
            // Allocate the first available register
            uint8_t phys_reg = *available.begin();
            available.erase(available.begin());
            
            // Map virtual to physical register
            reg_map[range.reg] = phys_reg;
            
            // Add to active set
            active.insert(&range);
        } else {
            // Spill the range with the furthest end point
            LiveRange* spill_range = *active.rbegin();
            
            if (spill_range->end > range.end) {
                // Spill the range with the furthest end point
                uint8_t phys_reg = reg_map[spill_range->reg];
                
                // Allocate a spill slot if needed
                if (spill_map.find(spill_range->reg) == spill_map.end()) {
                    spill_map[spill_range->reg] = next_spill_slot++;
                    spills++;
                }
                
                // Remove from active set
                active.erase(std::prev(active.end()));
                
                // Map virtual to physical register
                reg_map[range.reg] = phys_reg;
                
                // Add to active set
                active.insert(&range);
            } else {
                // Spill the current range
                // Allocate a spill slot
                spill_map[range.reg] = next_spill_slot++;
                spills++;
            }
        }
    }
    
    // Create register allocations for each instruction
    for (uint32_t i = 0; i < instruction_allocations.size(); i++) {
        // Find registers used by this instruction
        for (uint8_t reg = 0; reg < NUM_VITA_REGS; reg++) {
            if ((register_usage[reg].read && register_usage[reg].first_use <= i && i <= register_usage[reg].last_use) ||
                (register_usage[reg].written && register_usage[reg].first_use <= i && i <= register_usage[reg].last_use)) {
                // Create allocation
                RegisterAllocation alloc;
                alloc.vita_reg = reg;
                
                // Check if register is mapped or spilled
                if (reg_map.find(reg) != reg_map.end()) {
                    alloc.switch_reg = reg_map[reg];
                    alloc.spilled = false;
                } else if (spill_map.find(reg) != spill_map.end()) {
                    alloc.spilled = true;
                    alloc.spill_slot = spill_map[reg];
                    reloads++;
                }
                
                // Add to instruction allocations
                instruction_allocations[i].push_back(alloc);
            }
        }
    }
}

// RegisterAllocationOptimizer implementation
RegisterAllocationOptimizer::RegisterAllocationOptimizer()
    : total_blocks(0), optimized_blocks(0), total_spills(0), total_reloads(0) {
}

RegisterAllocationOptimizer::~RegisterAllocationOptimizer() {
}

void RegisterAllocationOptimizer::optimizeFunction(const std::vector<std::unique_ptr<BasicBlock>>& blocks) {
    // Reset statistics
    total_blocks = blocks.size();
    optimized_blocks = 0;
    total_spills = 0;
    total_reloads = 0;
    
    // Reset block allocators
    block_allocators.clear();
    
    // Analyze global register usage
    analyzeGlobalRegisterUsage(blocks);
    
    // Assign global registers
    assignGlobalRegisters();
    
    // Optimize each block
    for (const auto& block : blocks) {
        // Create a register allocator for this block
        auto allocator = std::make_unique<RegisterAllocator>();
        
        // Allocate registers for the block
        allocator->analyzeRegisterUsage(*block);
        
        // Get statistics
        uint32_t reads, writes, spills, reloads;
        allocator->getRegisterUsageStats(reads, writes, spills, reloads);
        
        // Update statistics
        total_spills += spills;
        total_reloads += reloads;
        
        // Store the allocator
        block_allocators[block->start_address] = std::move(allocator);
        
        // Count as optimized if no spills
        if (spills == 0) {
            optimized_blocks++;
        }
    }
    
    // Optimize block boundaries
    optimizeBlockBoundaries(blocks);
}

std::vector<RegisterAllocation> RegisterAllocationOptimizer::getBlockAllocation(uint32_t block_address) {
    auto it = block_allocators.find(block_address);
    if (it != block_allocators.end()) {
        // Get the first instruction's allocation
        return it->second->getInstructionAllocation(0);
    }
    return {};
}

void RegisterAllocationOptimizer::getAllocationStats(uint32_t& total_blocks, uint32_t& optimized_blocks, 
                                                  uint32_t& total_spills, uint32_t& total_reloads) {
    total_blocks = this->total_blocks;
    optimized_blocks = this->optimized_blocks;
    total_spills = this->total_spills;
    total_reloads = this->total_reloads;
}

void RegisterAllocationOptimizer::analyzeGlobalRegisterUsage(const std::vector<std::unique_ptr<BasicBlock>>& blocks) {
    // This is a stub implementation. In a real implementation, we would analyze
    // register usage across all blocks to identify global registers.
    
    // Reset global allocation
    global_allocation.clear();
    
    // For now, just reserve SP, LR, and PC
    global_allocation[RegisterAllocator::SP_REG] = RegisterAllocator::SP_REG;
    global_allocation[RegisterAllocator::LR_REG] = RegisterAllocator::LR_REG;
    global_allocation[RegisterAllocator::PC_REG] = RegisterAllocator::PC_REG;
}

void RegisterAllocationOptimizer::assignGlobalRegisters() {
    // This is a stub implementation. In a real implementation, we would assign
    // global registers based on the analysis.
    
    // For now, just use the reserved registers
}

void RegisterAllocationOptimizer::optimizeBlockBoundaries(const std::vector<std::unique_ptr<BasicBlock>>& blocks) {
    // This is a stub implementation. In a real implementation, we would optimize
    // register allocation at block boundaries to minimize spills and reloads.
    
    // For now, do nothing
}
