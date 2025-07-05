#include "syscall.h"
#include "kernel.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace firmware {

SyscallManager::SyscallManager(std::shared_ptr<Kernel> kernel)
    : kernel(kernel), logging_enabled(false) {
}

SyscallManager::~SyscallManager() {
}

bool SyscallManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    std::cout << "Initializing syscall manager" << std::endl;
    
    // Register default syscalls
    register_default_syscalls();
    
    std::cout << "Syscall manager initialized with " << syscalls.size() << " syscalls" << std::endl;
    return true;
}

bool SyscallManager::register_syscall(uint32_t syscall_id, const std::string &name, const std::string &description, SyscallHandler handler) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (syscalls.find(syscall_id) != syscalls.end()) {
        std::cerr << "Syscall " << syscall_id << " is already registered" << std::endl;
        return false;
    }
    
    SyscallInfo info;
    info.id = syscall_id;
    info.name = name;
    info.description = description;
    info.handler = handler;
    info.call_count = 0;
    
    syscalls[syscall_id] = info;
    
    std::cout << "Registered syscall " << syscall_id << ": " << name << std::endl;
    return true;
}

bool SyscallManager::unregister_syscall(uint32_t syscall_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = syscalls.find(syscall_id);
    if (it == syscalls.end()) {
        std::cerr << "Syscall " << syscall_id << " is not registered" << std::endl;
        return false;
    }
    
    syscalls.erase(it);
    
    std::cout << "Unregistered syscall " << syscall_id << std::endl;
    return true;
}

int SyscallManager::handle_syscall(uint32_t syscall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = syscalls.find(syscall_id);
    if (it == syscalls.end()) {
        std::cerr << "Syscall " << syscall_id << " is not registered" << std::endl;
        return -1;
    }
    
    // Increment call count
    it->second.call_count++;
    
    // Log syscall if enabled
    if (logging_enabled) {
        std::cout << "Syscall " << it->second.name << " (" << syscall_id << "): "
                  << "0x" << std::hex << arg1 << ", "
                  << "0x" << arg2 << ", "
                  << "0x" << arg3 << ", "
                  << "0x" << arg4 << std::dec << std::endl;
    }
    
    // Call handler
    return it->second.handler(arg1, arg2, arg3, arg4);
}

SyscallInfo SyscallManager::get_syscall_info(uint32_t syscall_id) const {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = syscalls.find(syscall_id);
    if (it == syscalls.end()) {
        SyscallInfo empty;
        empty.id = 0;
        return empty;
    }
    
    return it->second;
}

std::vector<SyscallInfo> SyscallManager::get_all_syscall_info() const {
    std::lock_guard<std::mutex> lock(mutex);
    
    std::vector<SyscallInfo> result;
    for (const auto &pair : syscalls) {
        result.push_back(pair.second);
    }
    
    return result;
}

void SyscallManager::get_syscall_statistics(std::vector<std::pair<uint32_t, uint32_t>> &stats) const {
    std::lock_guard<std::mutex> lock(mutex);
    
    stats.clear();
    for (const auto &pair : syscalls) {
        stats.push_back(std::make_pair(pair.first, pair.second.call_count));
    }
    
    // Sort by call count (descending)
    std::sort(stats.begin(), stats.end(),
        [](const std::pair<uint32_t, uint32_t> &a, const std::pair<uint32_t, uint32_t> &b) {
            return a.second > b.second;
        });
}

void SyscallManager::reset_syscall_statistics() {
    std::lock_guard<std::mutex> lock(mutex);
    
    for (auto &pair : syscalls) {
        pair.second.call_count = 0;
    }
    
    std::cout << "Reset syscall statistics" << std::endl;
}

void SyscallManager::enable_logging(bool enable) {
    std::lock_guard<std::mutex> lock(mutex);
    
    logging_enabled = enable;
    
    std::cout << "Syscall logging " << (enable ? "enabled" : "disabled") << std::endl;
}

bool SyscallManager::is_logging_enabled() const {
    std::lock_guard<std::mutex> lock(mutex);
    
    return logging_enabled;
}

void SyscallManager::register_default_syscalls() {
    // Process syscalls
    register_syscall(0x1001, "sceKernelCreateProcess", "Create a new process", process_syscalls::create_process);
    register_syscall(0x1002, "sceKernelExitProcess", "Exit the current process", process_syscalls::exit_process);
    register_syscall(0x1003, "sceKernelGetProcessId", "Get the current process ID", process_syscalls::get_process_id);
    register_syscall(0x1004, "sceKernelGetParentProcessId", "Get the parent process ID", process_syscalls::get_parent_process_id);
    register_syscall(0x1005, "sceKernelGetProcessName", "Get the process name", process_syscalls::get_process_name);
    
    // Thread syscalls
    register_syscall(0x2001, "sceKernelCreateThread", "Create a new thread", thread_syscalls::create_thread);
    register_syscall(0x2002, "sceKernelExitThread", "Exit the current thread", thread_syscalls::exit_thread);
    register_syscall(0x2003, "sceKernelStartThread", "Start a thread", thread_syscalls::start_thread);
    register_syscall(0x2004, "sceKernelStopThread", "Stop a thread", thread_syscalls::stop_thread);
    register_syscall(0x2005, "sceKernelSleepThread", "Sleep the current thread", thread_syscalls::sleep_thread);
    register_syscall(0x2006, "sceKernelGetThreadId", "Get the current thread ID", thread_syscalls::get_thread_id);
    register_syscall(0x2007, "sceKernelGetThreadName", "Get the thread name", thread_syscalls::get_thread_name);
    
    // Memory syscalls
    register_syscall(0x3001, "sceKernelAllocMemBlock", "Allocate a memory block", memory_syscalls::allocate_memory);
    register_syscall(0x3002, "sceKernelFreeMemBlock", "Free a memory block", memory_syscalls::free_memory);
    register_syscall(0x3003, "sceKernelGetMemBlockInfo", "Get memory block information", memory_syscalls::get_memory_info);
    register_syscall(0x3004, "sceKernelProtectMemBlock", "Change memory block protection", memory_syscalls::protect_memory);
    
    // Synchronization syscalls
    register_syscall(0x4001, "sceKernelCreateSema", "Create a semaphore", sync_syscalls::create_semaphore);
    register_syscall(0x4002, "sceKernelDeleteSema", "Delete a semaphore", sync_syscalls::delete_semaphore);
    register_syscall(0x4003, "sceKernelSignalSema", "Signal a semaphore", sync_syscalls::signal_semaphore);
    register_syscall(0x4004, "sceKernelWaitSema", "Wait for a semaphore", sync_syscalls::wait_semaphore);
    
    register_syscall(0x4101, "sceKernelCreateMutex", "Create a mutex", sync_syscalls::create_mutex);
    register_syscall(0x4102, "sceKernelDeleteMutex", "Delete a mutex", sync_syscalls::delete_mutex);
    register_syscall(0x4103, "sceKernelLockMutex", "Lock a mutex", sync_syscalls::lock_mutex);
    register_syscall(0x4104, "sceKernelUnlockMutex", "Unlock a mutex", sync_syscalls::unlock_mutex);
    
    register_syscall(0x4201, "sceKernelCreateEventFlag", "Create an event flag", sync_syscalls::create_event_flag);
    register_syscall(0x4202, "sceKernelDeleteEventFlag", "Delete an event flag", sync_syscalls::delete_event_flag);
    register_syscall(0x4203, "sceKernelSetEventFlag", "Set an event flag", sync_syscalls::set_event_flag);
    register_syscall(0x4204, "sceKernelClearEventFlag", "Clear an event flag", sync_syscalls::clear_event_flag);
    register_syscall(0x4205, "sceKernelWaitEventFlag", "Wait for an event flag", sync_syscalls::wait_event_flag);
    
    // File syscalls
    register_syscall(0x5001, "sceIoOpen", "Open a file", file_syscalls::open_file);
    register_syscall(0x5002, "sceIoClose", "Close a file", file_syscalls::close_file);
    register_syscall(0x5003, "sceIoRead", "Read from a file", file_syscalls::read_file);
    register_syscall(0x5004, "sceIoWrite", "Write to a file", file_syscalls::write_file);
    register_syscall(0x5005, "sceIoLseek", "Seek in a file", file_syscalls::seek_file);
    register_syscall(0x5006, "sceIoStat", "Get file status", file_syscalls::stat_file);
    register_syscall(0x5007, "sceIoRemove", "Remove a file", file_syscalls::remove_file);
    register_syscall(0x5008, "sceIoMkdir", "Create a directory", file_syscalls::mkdir);
    register_syscall(0x5009, "sceIoRmdir", "Remove a directory", file_syscalls::rmdir);
    
    // Module syscalls
    register_syscall(0x6001, "sceKernelLoadModule", "Load a module", module_syscalls::load_module);
    register_syscall(0x6002, "sceKernelUnloadModule", "Unload a module", module_syscalls::unload_module);
    register_syscall(0x6003, "sceKernelStartModule", "Start a module", module_syscalls::start_module);
    register_syscall(0x6004, "sceKernelStopModule", "Stop a module", module_syscalls::stop_module);
    register_syscall(0x6005, "sceKernelGetModuleInfo", "Get module information", module_syscalls::get_module_info);
    
    // Debug syscalls
    register_syscall(0x7001, "sceKernelPrintDebug", "Print debug message", debug_syscalls::print_debug);
    register_syscall(0x7002, "sceKernelAssertFail", "Assert failure", debug_syscalls::assert_fail);
    register_syscall(0x7003, "sceKernelGetCpuRegisters", "Get CPU registers", debug_syscalls::get_cpu_registers);
    register_syscall(0x7004, "sceKernelSetBreakpoint", "Set a breakpoint", debug_syscalls::set_breakpoint);
    register_syscall(0x7005, "sceKernelClearBreakpoint", "Clear a breakpoint", debug_syscalls::clear_breakpoint);
}

} // namespace firmware
