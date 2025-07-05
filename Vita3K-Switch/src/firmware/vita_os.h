#pragma once

#include "kernel.h"
#include "module.h"
#include "service.h"
#include "memory.h"
#include "syscall.h"
#include "pup.h"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace firmware {

// Forward declarations
class VitaOS;

// OS status
enum class OSStatus {
    UNINITIALIZED,
    INITIALIZING,
    RUNNING,
    SHUTTING_DOWN,
    SHUTDOWN
};

// OS initialization parameters
struct OSInitParams {
    std::string firmware_path;
    std::string app_path;
    std::string save_path;
    bool log_imports = false;
    bool log_exports = false;
    bool log_syscalls = false;
    bool log_memory = false;
    bool log_threads = false;
};

// OS class
class VitaOS {
public:
    VitaOS();
    ~VitaOS();

    // Initialize the OS
    bool initialize(const OSInitParams &params);

    // Start the OS
    bool start();

    // Stop the OS
    bool stop();

    // Get OS status
    OSStatus get_status() const;

    // Load a firmware package
    bool load_firmware(const std::string &pup_path);

    // Load an application
    bool load_application(const std::string &app_path);

    // Run an application
    bool run_application(const std::string &app_path);

    // Get kernel
    std::shared_ptr<Kernel> get_kernel() const;

    // Get module manager
    std::shared_ptr<ModuleManager> get_module_manager() const;

    // Get service manager
    std::shared_ptr<ServiceManager> get_service_manager() const;

    // Get memory manager
    std::shared_ptr<MemoryManager> get_memory_manager() const;

    // Get syscall manager
    std::shared_ptr<SyscallManager> get_syscall_manager() const;

    // Get firmware version
    std::string get_firmware_version() const;

    // Set progress callback
    void set_progress_callback(const std::function<void(uint32_t)> &callback);

    // Handle a system call
    int handle_syscall(uint32_t syscall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

private:
    // Initialize kernel
    bool initialize_kernel();

    // Initialize modules
    bool initialize_modules();

    // Initialize services
    bool initialize_services();

    // Initialize memory
    bool initialize_memory();

    // Initialize syscalls
    bool initialize_syscalls();

    // Load system modules
    bool load_system_modules();

    // OS status
    OSStatus status;

    // Kernel
    std::shared_ptr<Kernel> kernel;

    // Module manager
    std::shared_ptr<ModuleManager> module_manager;

    // Service manager
    std::shared_ptr<ServiceManager> service_manager;

    // Memory manager
    std::shared_ptr<MemoryManager> memory_manager;

    // Syscall manager
    std::shared_ptr<SyscallManager> syscall_manager;

    // Firmware version
    std::string firmware_version;

    // Initialization parameters
    OSInitParams params;

    // Progress callback
    std::function<void(uint32_t)> progress_callback;
};

} // namespace firmware
