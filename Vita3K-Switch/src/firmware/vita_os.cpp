#include "vita_os.h"
#include <iostream>
#include <filesystem>
#include <cstring>
#include <spdlog/spdlog.h>
#include <vector>
#include <string>
#include "pup_extract.h"

namespace fs = std::filesystem;
namespace firmware {

VitaOS::VitaOS()
    : status(OSStatus::UNINITIALIZED) {
}

VitaOS::~VitaOS() {
    if (status == OSStatus::RUNNING) {
        stop();
    }
}

bool VitaOS::initialize(const OSInitParams &params) {
    if (status != OSStatus::UNINITIALIZED) {
        std::cerr << "VitaOS is already initialized" << std::endl;
        return false;
    }

    std::cout << "Initializing VitaOS" << std::endl;
    status = OSStatus::INITIALIZING;
    this->params = params;

    // Create kernel, module manager, service manager, memory manager, and syscall manager
    kernel = std::make_shared<Kernel>();
    module_manager = std::make_shared<ModuleManager>();
    service_manager = std::make_shared<ServiceManager>();
    memory_manager = std::make_shared<MemoryManager>();
    syscall_manager = std::make_shared<SyscallManager>(kernel);

    // Initialize kernel
    if (!initialize_kernel()) {
        std::cerr << "Failed to initialize kernel" << std::endl;
        status = OSStatus::UNINITIALIZED;
        return false;
    }

    // Initialize modules
    if (!initialize_modules()) {
        std::cerr << "Failed to initialize modules" << std::endl;
        status = OSStatus::UNINITIALIZED;
        return false;
    }

    // Initialize services
    if (!initialize_services()) {
        std::cerr << "Failed to initialize services" << std::endl;
        status = OSStatus::UNINITIALIZED;
        return false;
    }

    // Initialize memory
    if (!initialize_memory()) {
        std::cerr << "Failed to initialize memory" << std::endl;
        status = OSStatus::UNINITIALIZED;
        return false;
    }

    // Initialize syscalls
    if (!initialize_syscalls()) {
        std::cerr << "Failed to initialize syscalls" << std::endl;
        status = OSStatus::UNINITIALIZED;
        return false;
    }

    // Load firmware if specified
    if (!params.firmware_path.empty()) {
        if (!load_firmware(params.firmware_path)) {
            std::cerr << "Failed to load firmware" << std::endl;
            status = OSStatus::UNINITIALIZED;
            return false;
        }
    }

    std::cout << "VitaOS initialized successfully" << std::endl;
    return true;
}

bool VitaOS::start() {
    if (status != OSStatus::INITIALIZING) {
        std::cerr << "VitaOS is not initialized" << std::endl;
        return false;
    }

    std::cout << "Starting VitaOS" << std::endl;

    // Start services
    if (!service_manager->start_all_services()) {
        std::cerr << "Failed to start services" << std::endl;
        return false;
    }

    // Load system modules
    if (!load_system_modules()) {
        std::cerr << "Failed to load system modules" << std::endl;
        return false;
    }

    // Load application if specified
    if (!params.app_path.empty()) {
        if (!load_application(params.app_path)) {
            std::cerr << "Failed to load application" << std::endl;
            return false;
        }
    }

    status = OSStatus::RUNNING;
    std::cout << "VitaOS started successfully" << std::endl;
    return true;
}

bool VitaOS::stop() {
    if (status != OSStatus::RUNNING) {
        std::cerr << "VitaOS is not running" << std::endl;
        return false;
    }

    std::cout << "Stopping VitaOS" << std::endl;
    status = OSStatus::SHUTTING_DOWN;

    // Stop services
    service_manager->stop_all_services();

    // Unload all modules
    module_manager->unload_all_modules();

    status = OSStatus::SHUTDOWN;
    std::cout << "VitaOS stopped successfully" << std::endl;
    return true;
}

OSStatus VitaOS::get_status() const {
    return status;
}

bool VitaOS::load_firmware(const std::string &pup_path) {
    if (status != OSStatus::INITIALIZING && status != OSStatus::RUNNING) {
        std::cerr << "VitaOS is not initialized" << std::endl;
        return false;
    }

    std::cout << "Loading firmware from " << pup_path << std::endl;

    // Install PUP file
    std::string version = install_pup(params.firmware_path, pup_path, progress_callback);
    if (version.empty()) {
        std::cerr << "Failed to install PUP file" << std::endl;
        return false;
    }

    firmware_version = version;
    std::cout << "Firmware version: " << firmware_version << std::endl;
    return true;
}

bool VitaOS::load_application(const std::string &app_path) {
    if (status != OSStatus::INITIALIZING && status != OSStatus::RUNNING) {
        std::cerr << "VitaOS is not initialized" << std::endl;
        return false;
    }

    std::cout << "Loading application from " << app_path << std::endl;

    // Create a process for the application
    auto process = kernel->create_process("Application");
    if (!process) {
        std::cerr << "Failed to create process for application" << std::endl;
        return false;
    }

    // Load the application module
    auto module = module_manager->load_module(app_path, ModuleType::USER);
    if (!module) {
        std::cerr << "Failed to load application module" << std::endl;
        return false;
    }

    std::cout << "Application loaded successfully" << std::endl;
    return true;
}

bool VitaOS::run_application(const std::string &app_path) {
    if (status != OSStatus::RUNNING) {
        std::cerr << "VitaOS is not running" << std::endl;
        return false;
    }

    // Load the application if not already loaded
    if (!load_application(app_path)) {
        return false;
    }

    std::cout << "Running application from " << app_path << std::endl;

    // Get the application process
    auto processes = kernel->get_all_processes();
    std::shared_ptr<Process> app_process = nullptr;
    for (auto &process : processes) {
        if (process->get_name() == "Application") {
            app_process = process;
            break;
        }
    }

    if (!app_process) {
        std::cerr << "Application process not found" << std::endl;
        return false;
    }

    // Start the application process
    if (!app_process->start()) {
        std::cerr << "Failed to start application process" << std::endl;
        return false;
    }

    std::cout << "Application started successfully" << std::endl;
    return true;
}

std::shared_ptr<Kernel> VitaOS::get_kernel() const {
    return kernel;
}

std::shared_ptr<ModuleManager> VitaOS::get_module_manager() const {
    return module_manager;
}

std::shared_ptr<ServiceManager> VitaOS::get_service_manager() const {
    return service_manager;
}

std::shared_ptr<MemoryManager> VitaOS::get_memory_manager() const {
    return memory_manager;
}

std::shared_ptr<SyscallManager> VitaOS::get_syscall_manager() const {
    return syscall_manager;
}

std::string VitaOS::get_firmware_version() const {
    return firmware_version;
}

void VitaOS::set_progress_callback(const std::function<void(uint32_t)> &callback) {
    progress_callback = callback;
}

int VitaOS::handle_syscall(uint32_t syscall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    if (status != OSStatus::RUNNING) {
        std::cerr << "VitaOS is not running" << std::endl;
        return -1;
    }
    
    return syscall_manager->handle_syscall(syscall_id, arg1, arg2, arg3, arg4);
}

// Scan and load required firmware modules from SD card
static bool scan_and_load_firmware_modules(const std::string& firmware_root, std::shared_ptr<ModuleManager> module_manager) {
    namespace fs = std::filesystem;
    std::string modules_dir = firmware_root + "/vs0/modules/";
    std::vector<std::string> required_modules = {
        "libkernel.suprx",
        "libc.suprx",
        "libSceSysmem.suprx",
        "libSceThreadmgr.suprx",
        "libSceDisplay.suprx",
        // ... add more as needed ...
    };
    bool all_ok = true;
    for (const auto& mod : required_modules) {
        fs::path mod_path = modules_dir + mod;
        if (!fs::exists(mod_path)) {
            spdlog::error("Missing required module: {}", mod_path.string());
            all_ok = false;
            continue;
        }
        if (!module_manager->load_module_by_path(mod_path, ModuleType::SYSTEM)) {
            spdlog::error("Failed to load module: {}", mod_path.string());
            all_ok = false;
        } else {
            spdlog::info("Loaded module: {}", mod_path.string());
        }
    }
    return all_ok;
}

bool VitaOS::initialize_kernel() {
    std::cout << "Initializing kernel" << std::endl;
    
    if (!kernel->initialize()) {
        std::cerr << "Failed to initialize kernel" << std::endl;
        return false;
    }

    // Scan and load firmware modules from SD card
    std::string firmware_root = "sdmc:/switch/vitans/firmware";
    if (!scan_and_load_firmware_modules(firmware_root, module_manager)) {
        spdlog::error("One or more required firmware modules failed to load. Emulator may not function correctly.");
    }

    // Register system call handlers
    kernel->register_syscall(0x1, [](uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
        std::cout << "System call: Process create" << std::endl;
        return 0;
    });

    kernel->register_syscall(0x2, [](uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
        std::cout << "System call: Process exit" << std::endl;
        return 0;
    });

    kernel->register_syscall(0x3, [](uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
        std::cout << "System call: Thread create" << std::endl;
        return 0;
    });

    kernel->register_syscall(0x4, [](uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
        std::cout << "System call: Thread exit" << std::endl;
        return 0;
    });

    std::cout << "Kernel initialized successfully" << std::endl;
    return true;
}

bool VitaOS::initialize_modules() {
    std::cout << "Initializing module manager" << std::endl;
    
    // Set module paths
    module_manager->set_system_module_path(params.firmware_path + "/system/modules");
    module_manager->set_user_module_path(params.firmware_path + "/user/modules");
    
    std::cout << "Module manager initialized successfully" << std::endl;
    return true;
}

bool VitaOS::initialize_services() {
    std::cout << "Initializing service manager" << std::endl;
    
    if (!service_manager->initialize()) {
        std::cerr << "Failed to initialize service manager" << std::endl;
        return false;
    }

    // Create and register system services
    auto network_service = std::make_shared<NetworkService>();
    if (!network_service->initialize()) {
        std::cerr << "Failed to initialize network service" << std::endl;
        return false;
    }
    service_manager->register_service(network_service);

    auto file_system_service = std::make_shared<FileSystemService>();
    if (!file_system_service->initialize()) {
        std::cerr << "Failed to initialize file system service" << std::endl;
        return false;
    }
    service_manager->register_service(file_system_service);

    auto audio_service = std::make_shared<AudioService>();
    if (!audio_service->initialize()) {
        std::cerr << "Failed to initialize audio service" << std::endl;
        return false;
    }
    service_manager->register_service(audio_service);

    auto input_service = std::make_shared<InputService>();
    if (!input_service->initialize()) {
        std::cerr << "Failed to initialize input service" << std::endl;
        return false;
    }
    service_manager->register_service(input_service);

    auto display_service = std::make_shared<DisplayService>();
    if (!display_service->initialize()) {
        std::cerr << "Failed to initialize display service" << std::endl;
        return false;
    }
    service_manager->register_service(display_service);

    std::cout << "Service manager initialized successfully" << std::endl;
    return true;
}

bool VitaOS::initialize_memory() {
    std::cout << "Initializing memory manager" << std::endl;
    
    // Initialize memory manager with 512MB of memory (typical for PS Vita)
    const uint64_t VITA_MEMORY_SIZE = 512 * 1024 * 1024;
    if (!memory_manager->initialize(VITA_MEMORY_SIZE)) {
        std::cerr << "Failed to initialize memory manager" << std::endl;
        return false;
    }
    
    // Allocate kernel memory
    auto kernel_mem = memory_manager->allocate_memory(
        0, // Kernel process ID
        64 * 1024 * 1024, // 64MB for kernel
        MemoryPermission::READ_WRITE_EXECUTE,
        MemoryType::KERNEL,
        "KernelMemory"
    );
    
    if (!kernel_mem) {
        std::cerr << "Failed to allocate kernel memory" << std::endl;
        return false;
    }
    
    // Allocate user memory
    auto user_mem = memory_manager->allocate_memory(
        0, // Will be assigned to processes later
        384 * 1024 * 1024, // 384MB for user
        MemoryPermission::READ_WRITE,
        MemoryType::USER,
        "UserMemory"
    );
    
    if (!user_mem) {
        std::cerr << "Failed to allocate user memory" << std::endl;
        return false;
    }
    
    // Allocate VRAM
    auto vram = memory_manager->allocate_memory(
        0, // Kernel process ID
        128 * 1024 * 1024, // 128MB for VRAM
        MemoryPermission::READ_WRITE,
        MemoryType::VRAM,
        "VRAM"
    );
    
    if (!vram) {
        std::cerr << "Failed to allocate VRAM" << std::endl;
        return false;
    }
    
    std::cout << "Memory manager initialized successfully" << std::endl;
    memory_manager->dump_memory_map();
    return true;
}

bool VitaOS::initialize_syscalls() {
    std::cout << "Initializing syscall manager" << std::endl;
    
    if (!syscall_manager->initialize()) {
        std::cerr << "Failed to initialize syscall manager" << std::endl;
        return false;
    }
    
    // Enable syscall logging if requested
    if (params.log_syscalls) {
        syscall_manager->enable_logging(true);
    }
    
    std::cout << "Syscall manager initialized successfully" << std::endl;
    return true;
}

bool VitaOS::load_system_modules() {
    std::cout << "Loading system modules" << std::endl;

    // Create system modules directory if it doesn't exist
    fs::path system_module_path = params.firmware_path + "/system/modules";
    if (!fs::exists(system_module_path)) {
        if (!fs::create_directories(system_module_path)) {
            std::cerr << "Failed to create system modules directory" << std::endl;
            return false;
        }
    }

    // Load essential system modules
    const std::vector<std::string> essential_modules = {
        "SceKernelModulemgr",
        "SceIofilemgr",
        "SceLibKernel",
        "SceProcessmgr",
        "SceThreadmgr"
    };

    for (const auto &module_name : essential_modules) {
        std::cout << "Loading system module: " << module_name << std::endl;
        auto module = module_manager->load_module(module_name, ModuleType::SYSTEM);
        if (!module) {
            std::cerr << "Failed to load system module: " << module_name << std::endl;
            // Continue loading other modules even if one fails
        }
    }

    std::cout << "System modules loaded successfully" << std::endl;
    return true;
}

// Install firmware from a .PUP file on the SD card
bool install_firmware_from_pup(const std::string& pup_path) {
    std::string firmware_root = "sdmc:/switch/vitans/firmware";
    spdlog::info("Installing firmware from PUP: {} to {}", pup_path, firmware_root);
    if (!extract_pup_firmware(pup_path, firmware_root)) {
        spdlog::error("Failed to extract PUP firmware.");
        return false;
    }
    spdlog::info("Firmware installation from PUP complete.");
    return true;
}

} // namespace firmware
