#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <mutex>

namespace firmware {

// Forward declarations
class Kernel;
class Process;
class Thread;

// Syscall handler type
using SyscallHandler = std::function<int(uint32_t, uint32_t, uint32_t, uint32_t)>;

// Syscall info structure
struct SyscallInfo {
    uint32_t id;
    std::string name;
    std::string description;
    SyscallHandler handler;
    uint32_t call_count;
};

// Syscall manager class
class SyscallManager {
public:
    SyscallManager(std::shared_ptr<Kernel> kernel);
    ~SyscallManager();

    // Initialize the syscall manager
    bool initialize();

    // Register a syscall
    bool register_syscall(uint32_t syscall_id, const std::string &name, const std::string &description, SyscallHandler handler);

    // Unregister a syscall
    bool unregister_syscall(uint32_t syscall_id);

    // Handle a syscall
    int handle_syscall(uint32_t syscall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

    // Get syscall info
    SyscallInfo get_syscall_info(uint32_t syscall_id) const;

    // Get all syscall info
    std::vector<SyscallInfo> get_all_syscall_info() const;

    // Get syscall statistics
    void get_syscall_statistics(std::vector<std::pair<uint32_t, uint32_t>> &stats) const;

    // Reset syscall statistics
    void reset_syscall_statistics();

    // Enable syscall logging
    void enable_logging(bool enable);

    // Check if syscall logging is enabled
    bool is_logging_enabled() const;

private:
    // Register default syscalls
    void register_default_syscalls();

    // Kernel reference
    std::shared_ptr<Kernel> kernel;

    // Syscall handlers
    std::map<uint32_t, SyscallInfo> syscalls;

    // Logging enabled flag
    bool logging_enabled;

    // Mutex for thread safety
    mutable std::mutex mutex;
};

// Process syscalls
namespace process_syscalls {
    int create_process(uint32_t name_ptr, uint32_t args_ptr, uint32_t flags, uint32_t process_id_ptr);
    int exit_process(uint32_t exit_code, uint32_t, uint32_t, uint32_t);
    int get_process_id(uint32_t process_id_ptr, uint32_t, uint32_t, uint32_t);
    int get_parent_process_id(uint32_t process_id_ptr, uint32_t, uint32_t, uint32_t);
    int get_process_name(uint32_t process_id, uint32_t name_ptr, uint32_t name_len, uint32_t);
}

// Thread syscalls
namespace thread_syscalls {
    int create_thread(uint32_t name_ptr, uint32_t entry_ptr, uint32_t args_ptr, uint32_t flags);
    int exit_thread(uint32_t exit_code, uint32_t, uint32_t, uint32_t);
    int start_thread(uint32_t thread_id, uint32_t, uint32_t, uint32_t);
    int stop_thread(uint32_t thread_id, uint32_t, uint32_t, uint32_t);
    int sleep_thread(uint32_t microseconds, uint32_t, uint32_t, uint32_t);
    int get_thread_id(uint32_t thread_id_ptr, uint32_t, uint32_t, uint32_t);
    int get_thread_name(uint32_t thread_id, uint32_t name_ptr, uint32_t name_len, uint32_t);
}

// Memory syscalls
namespace memory_syscalls {
    int allocate_memory(uint32_t size, uint32_t flags, uint32_t alignment, uint32_t addr_ptr);
    int free_memory(uint32_t addr, uint32_t, uint32_t, uint32_t);
    int get_memory_info(uint32_t addr, uint32_t info_ptr, uint32_t, uint32_t);
    int protect_memory(uint32_t addr, uint32_t size, uint32_t prot, uint32_t);
}

// Synchronization syscalls
namespace sync_syscalls {
    int create_semaphore(uint32_t name_ptr, uint32_t attr_ptr, uint32_t initial_count, uint32_t max_count);
    int delete_semaphore(uint32_t semaphore_id, uint32_t, uint32_t, uint32_t);
    int signal_semaphore(uint32_t semaphore_id, uint32_t signal_count, uint32_t, uint32_t);
    int wait_semaphore(uint32_t semaphore_id, uint32_t signal_count, uint32_t timeout_ptr, uint32_t);
    
    int create_mutex(uint32_t name_ptr, uint32_t attr_ptr, uint32_t initial_count, uint32_t);
    int delete_mutex(uint32_t mutex_id, uint32_t, uint32_t, uint32_t);
    int lock_mutex(uint32_t mutex_id, uint32_t lock_count, uint32_t timeout_ptr, uint32_t);
    int unlock_mutex(uint32_t mutex_id, uint32_t unlock_count, uint32_t, uint32_t);
    
    int create_event_flag(uint32_t name_ptr, uint32_t attr_ptr, uint32_t initial_pattern, uint32_t);
    int delete_event_flag(uint32_t event_id, uint32_t, uint32_t, uint32_t);
    int set_event_flag(uint32_t event_id, uint32_t pattern, uint32_t, uint32_t);
    int clear_event_flag(uint32_t event_id, uint32_t pattern, uint32_t, uint32_t);
    int wait_event_flag(uint32_t event_id, uint32_t pattern, uint32_t wait_mode, uint32_t timeout_ptr);
}

// File syscalls
namespace file_syscalls {
    int open_file(uint32_t path_ptr, uint32_t flags, uint32_t mode, uint32_t);
    int close_file(uint32_t fd, uint32_t, uint32_t, uint32_t);
    int read_file(uint32_t fd, uint32_t buf_ptr, uint32_t size, uint32_t);
    int write_file(uint32_t fd, uint32_t buf_ptr, uint32_t size, uint32_t);
    int seek_file(uint32_t fd, uint32_t offset, uint32_t whence, uint32_t);
    int stat_file(uint32_t path_ptr, uint32_t stat_ptr, uint32_t, uint32_t);
    int remove_file(uint32_t path_ptr, uint32_t, uint32_t, uint32_t);
    int mkdir(uint32_t path_ptr, uint32_t mode, uint32_t, uint32_t);
    int rmdir(uint32_t path_ptr, uint32_t, uint32_t, uint32_t);
}

// Module syscalls
namespace module_syscalls {
    int load_module(uint32_t path_ptr, uint32_t flags, uint32_t, uint32_t);
    int unload_module(uint32_t module_id, uint32_t, uint32_t, uint32_t);
    int start_module(uint32_t module_id, uint32_t args_size, uint32_t args_ptr, uint32_t status_ptr);
    int stop_module(uint32_t module_id, uint32_t args_size, uint32_t args_ptr, uint32_t status_ptr);
    int get_module_info(uint32_t module_id, uint32_t info_ptr, uint32_t, uint32_t);
}

// Debug syscalls
namespace debug_syscalls {
    int print_debug(uint32_t message_ptr, uint32_t, uint32_t, uint32_t);
    int assert_fail(uint32_t condition_ptr, uint32_t file_ptr, uint32_t line, uint32_t);
    int get_cpu_registers(uint32_t thread_id, uint32_t regs_ptr, uint32_t, uint32_t);
    int set_breakpoint(uint32_t addr, uint32_t, uint32_t, uint32_t);
    int clear_breakpoint(uint32_t addr, uint32_t, uint32_t, uint32_t);
}

} // namespace firmware
