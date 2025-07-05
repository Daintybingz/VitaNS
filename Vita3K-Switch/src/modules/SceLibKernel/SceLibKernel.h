#pragma once

#include "../module.h"
#include <memory>

// SceLibKernel module - provides basic kernel functions
class SceLibKernel : public Module {
public:
    SceLibKernel();
    ~SceLibKernel();
    
    // Initialize the module
    bool initialize(Emulator& emulator) override;
    
    // Finalize the module
    void finalize(Emulator& emulator) override;
    
    // Create module instance
    static std::shared_ptr<Module> create();

private:
    // Register all functions
    void registerFunctions();
};

// Thread related structures
struct SceKernelThreadInfo {
    uint32_t size;
    char name[32];
    uint32_t attr;
    uint32_t status;
    uint32_t entry;
    uint32_t stack;
    uint32_t stackSize;
    uint32_t initPriority;
    uint32_t currentPriority;
    uint32_t waitType;
    uint32_t waitId;
    uint32_t exitStatus;
    uint64_t runClocks;
    uint32_t intrPreemptCount;
    uint32_t threadPreemptCount;
    uint32_t threadReleaseCount;
    uint32_t fNotifyCallback;
    uint32_t reserved[9];
};

// Thread related constants
constexpr uint32_t SCE_KERNEL_THREAD_ATTR_AFFINITY_MASK = 0x00000300;
constexpr uint32_t SCE_KERNEL_THREAD_ATTR_AFFINITY_SHIFT = 8;
constexpr uint32_t SCE_KERNEL_THREAD_ATTR_DETACHED = 0x00001000;
constexpr uint32_t SCE_KERNEL_THREAD_ATTR_VFPU = 0x00004000;

// Thread status constants
constexpr uint32_t SCE_KERNEL_THREAD_STATUS_DORMANT = 0x00000001;
constexpr uint32_t SCE_KERNEL_THREAD_STATUS_DEAD = 0x00000002;
constexpr uint32_t SCE_KERNEL_THREAD_STATUS_RUNNABLE = 0x00000004;
constexpr uint32_t SCE_KERNEL_THREAD_STATUS_READY = 0x00000008;
constexpr uint32_t SCE_KERNEL_THREAD_STATUS_RUNNING = 0x00000010;
constexpr uint32_t SCE_KERNEL_THREAD_STATUS_WAITING = 0x00000020;
constexpr uint32_t SCE_KERNEL_THREAD_STATUS_SUSPENDED = 0x00000040;
constexpr uint32_t SCE_KERNEL_THREAD_STATUS_STOPPED = 0x00000080;
constexpr uint32_t SCE_KERNEL_THREAD_STATUS_DELETED = 0x00000100;
constexpr uint32_t SCE_KERNEL_THREAD_STATUS_KILLED = 0x00000200;

// Memory allocation constants
constexpr uint32_t SCE_KERNEL_MEMBLOCK_TYPE_USER_RW = 0x0c20d060;
constexpr uint32_t SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE = 0x0c208060;
constexpr uint32_t SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_RW = 0x0c80d060;
constexpr uint32_t SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW = 0x0d808060;
constexpr uint32_t SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW = 0x09408060;

// Error codes
constexpr int SCE_KERNEL_ERROR_OK = 0;
constexpr int SCE_KERNEL_ERROR_ERROR = -1;
constexpr int SCE_KERNEL_ERROR_ILLEGAL_ADDR = 0x80000103;
constexpr int SCE_KERNEL_ERROR_ILLEGAL_ALIGNMENT = 0x80000104;
constexpr int SCE_KERNEL_ERROR_ILLEGAL_SIZE = 0x80000107;
constexpr int SCE_KERNEL_ERROR_ILLEGAL_PERMISSION = 0x8000010d;
constexpr int SCE_KERNEL_ERROR_INVALID_UID = 0x80000100;
constexpr int SCE_KERNEL_ERROR_ALREADY = 0x80000020;
constexpr int SCE_KERNEL_ERROR_BUSY = 0x80000021;
constexpr int SCE_KERNEL_ERROR_NO_MEMORY = 0x80000022;
constexpr int SCE_KERNEL_ERROR_TIMEOUT = 0x80000032;
