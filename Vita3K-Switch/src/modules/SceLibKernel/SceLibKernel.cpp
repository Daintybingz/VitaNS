#include "SceLibKernel.h"
#include "../../core/emulator/emulator.h"
#include <cstdio>
#include <cstring>
#include "core/memory/memory_manager.h"

// NIDs for SceLibKernel functions
constexpr NID NID_SCE_KERNEL_ALLOC_MEMORY_BLOCK = 0xB9D5EBDE;
constexpr NID NID_SCE_KERNEL_FREE_MEMORY_BLOCK = 0xA91E15EE;
constexpr NID NID_SCE_KERNEL_GET_MEMORY_BLOCK_BASE = 0xA841EDDA;
constexpr NID NID_SCE_KERNEL_CREATE_THREAD = 0xC5C11EE7;
constexpr NID NID_SCE_KERNEL_START_THREAD = 0x32BF938E;
constexpr NID NID_SCE_KERNEL_EXIT_THREAD = 0x0C8A38E1;
constexpr NID NID_SCE_KERNEL_DELAY_THREAD = 0x4B675D05;
constexpr NID NID_SCE_KERNEL_GET_THREAD_ID = 0x0FB972F9;
constexpr NID NID_SCE_KERNEL_GET_PROCESS_TIME = 0xF5D0D4C6;

// SceLibKernel implementation
SceLibKernel::SceLibKernel()
    : Module("SceLibKernel") {
    registerFunctions();
}

SceLibKernel::~SceLibKernel() {
}

bool SceLibKernel::initialize(Emulator& emulator) {
    printf("[SceLibKernel] Initializing\n");
    return true;
}

void SceLibKernel::finalize(Emulator& emulator) {
    printf("[SceLibKernel] Finalizing\n");
}

std::shared_ptr<Module> SceLibKernel::create() {
    return std::make_shared<SceLibKernel>();
}

// System call implementations

// Memory allocation
int sceKernelAllocMemBlock(Emulator& emulator, uint32_t threadId, const std::vector<uint32_t>& args) {
    if (args.size() < 2) {
        return SCE_KERNEL_ERROR_ILLEGAL_SIZE;
    }
    
    const char* name = reinterpret_cast<const char*>(args[0]);
    uint32_t type = args[1];
    uint32_t size = args[2];
    void* optp = args.size() > 3 ? reinterpret_cast<void*>(args[3]) : nullptr;
    
    printf("[SceLibKernel] sceKernelAllocMemBlock(name=%s, type=0x%08X, size=0x%X, optp=0x%p)\n",
        name, type, size, optp);
    
    // Validate parameters
    if (size == 0) {
        return SCE_KERNEL_ERROR_ILLEGAL_SIZE;
    }
    
    // Allocate memory using our memory manager
    MemoryManager& memoryManager = emulator.getMemoryManager();
    uint32_t addr = memoryManager.allocate(size, MemoryRegionType::RAM, 4096);
    if (addr == 0) {
        return SCE_KERNEL_ERROR_NO_MEMORY;
    }
    uint32_t uid = addr;
    printf("[SceLibKernel] Allocated memory block: UID=0x%08X, base=0x%08X, size=0x%X\n",
        uid, addr, size);
    return uid;
}

int sceKernelFreeMemBlock(Emulator& emulator, uint32_t threadId, const std::vector<uint32_t>& args) {
    if (args.size() < 1) {
        return SCE_KERNEL_ERROR_INVALID_UID;
    }
    
    uint32_t uid = args[0];
    
    printf("[SceLibKernel] sceKernelFreeMemBlock(uid=0x%08X)\n", uid);
    
    // Free the memory
    MemoryManager& memoryManager = emulator.getMemoryManager();
    bool success = memoryManager.free(uid);
    if (!success) {
        return SCE_KERNEL_ERROR_INVALID_UID;
    }
    printf("[SceLibKernel] Freed memory block: UID=0x%08X\n", uid);
    return SCE_KERNEL_ERROR_OK;
}

int sceKernelGetMemBlockBase(Emulator& emulator, uint32_t threadId, const std::vector<uint32_t>& args) {
    if (args.size() < 2) {
        return SCE_KERNEL_ERROR_INVALID_UID;
    }
    
    uint32_t uid = args[0];
    uint32_t basePtrAddr = args[1];
    
    printf("[SceLibKernel] sceKernelGetMemBlockBase(uid=0x%08X, basePtrAddr=0x%08X)\n", uid, basePtrAddr);
    
    // In a real implementation, we would look up the allocation by UID
    // For now, we'll just assume the UID is the base pointer
    void* base = reinterpret_cast<void*>(uid);
    
    // Write the base pointer to the provided address
    // In a real implementation, we would use the CPU to write to memory
    // For now, we'll just print what we would do
    printf("[SceLibKernel] Would write base pointer 0x%p to address 0x%08X\n", base, basePtrAddr);
    
    return SCE_KERNEL_ERROR_OK;
}

// Thread management
int sceKernelCreateThread(Emulator& emulator, uint32_t threadId, const std::vector<uint32_t>& args) {
    if (args.size() < 5) {
        return SCE_KERNEL_ERROR_ILLEGAL_SIZE;
    }
    
    const char* name = reinterpret_cast<const char*>(args[0]);
    uint32_t entry = args[1];
    uint32_t priority = args[2];
    uint32_t stackSize = args[3];
    uint32_t attr = args[4];
    uint32_t optionPtr = args.size() > 5 ? args[5] : 0;
    
    printf("[SceLibKernel] sceKernelCreateThread(name=%s, entry=0x%08X, priority=%d, stackSize=0x%X, attr=0x%08X, optionPtr=0x%08X)\n",
        name, entry, priority, stackSize, attr, optionPtr);
    
    // In a real implementation, we would create a thread
    // For now, we'll just return a dummy thread ID
    uint32_t newThreadId = 0x1000 + (threadId & 0xFF);
    
    printf("[SceLibKernel] Created thread: ID=0x%08X\n", newThreadId);
    
    return newThreadId;
}

int sceKernelStartThread(Emulator& emulator, uint32_t threadId, const std::vector<uint32_t>& args) {
    if (args.size() < 3) {
        return SCE_KERNEL_ERROR_INVALID_UID;
    }
    
    uint32_t targetThreadId = args[0];
    uint32_t argSize = args[1];
    uint32_t argPtr = args[2];
    
    printf("[SceLibKernel] sceKernelStartThread(threadId=0x%08X, argSize=%d, argPtr=0x%08X)\n",
        targetThreadId, argSize, argPtr);
    
    // In a real implementation, we would start the thread
    // For now, we'll just print what we would do
    printf("[SceLibKernel] Would start thread ID=0x%08X\n", targetThreadId);
    
    return SCE_KERNEL_ERROR_OK;
}

int sceKernelExitThread(Emulator& emulator, uint32_t threadId, const std::vector<uint32_t>& args) {
    if (args.size() < 1) {
        return SCE_KERNEL_ERROR_ILLEGAL_SIZE;
    }
    
    uint32_t exitStatus = args[0];
    
    printf("[SceLibKernel] sceKernelExitThread(exitStatus=0x%08X)\n", exitStatus);
    
    // In a real implementation, we would exit the thread
    // For now, we'll just print what we would do
    printf("[SceLibKernel] Would exit thread ID=0x%08X with status=0x%08X\n", threadId, exitStatus);
    
    return SCE_KERNEL_ERROR_OK;
}

int sceKernelDelayThread(Emulator& emulator, uint32_t threadId, const std::vector<uint32_t>& args) {
    if (args.size() < 1) {
        return SCE_KERNEL_ERROR_ILLEGAL_SIZE;
    }
    
    uint32_t delayUs = args[0];
    
    printf("[SceLibKernel] sceKernelDelayThread(delayUs=%d)\n", delayUs);
    
    // In a real implementation, we would delay the thread
    // For now, we'll just print what we would do
    printf("[SceLibKernel] Would delay thread ID=0x%08X for %d microseconds\n", threadId, delayUs);
    
    return SCE_KERNEL_ERROR_OK;
}

int sceKernelGetThreadId(Emulator& emulator, uint32_t threadId, const std::vector<uint32_t>& args) {
    printf("[SceLibKernel] sceKernelGetThreadId()\n");
    
    // Just return the current thread ID
    return threadId;
}

int sceKernelGetProcessTime(Emulator& emulator, uint32_t threadId, const std::vector<uint32_t>& args) {
    printf("[SceLibKernel] sceKernelGetProcessTime()\n");
    
    // In a real implementation, we would return the process time
    // For now, we'll just return a dummy value
    return 1000;
}

void SceLibKernel::registerFunctions() {
    // Memory management
    registerFunction("sceKernelAllocMemBlock", NID_SCE_KERNEL_ALLOC_MEMORY_BLOCK, sceKernelAllocMemBlock, 4);
    registerFunction("sceKernelFreeMemBlock", NID_SCE_KERNEL_FREE_MEMORY_BLOCK, sceKernelFreeMemBlock, 1);
    registerFunction("sceKernelGetMemBlockBase", NID_SCE_KERNEL_GET_MEMORY_BLOCK_BASE, sceKernelGetMemBlockBase, 2);
    
    // Thread management
    registerFunction("sceKernelCreateThread", NID_SCE_KERNEL_CREATE_THREAD, sceKernelCreateThread, 6);
    registerFunction("sceKernelStartThread", NID_SCE_KERNEL_START_THREAD, sceKernelStartThread, 3);
    registerFunction("sceKernelExitThread", NID_SCE_KERNEL_EXIT_THREAD, sceKernelExitThread, 1);
    registerFunction("sceKernelDelayThread", NID_SCE_KERNEL_DELAY_THREAD, sceKernelDelayThread, 1);
    registerFunction("sceKernelGetThreadId", NID_SCE_KERNEL_GET_THREAD_ID, sceKernelGetThreadId, 0);
    registerFunction("sceKernelGetProcessTime", NID_SCE_KERNEL_GET_PROCESS_TIME, sceKernelGetProcessTime, 0);
}
