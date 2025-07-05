#include "service.h"
#include <iostream>

namespace firmware {

// Service implementation
Service::Service(const std::string &name)
    : name(name), status(ServiceStatus::STOPPED) {
}

Service::~Service() {
    if (status == ServiceStatus::RUNNING) {
        stop();
    }
}

bool Service::start() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (status == ServiceStatus::RUNNING) {
        std::cerr << "Service " << name << " is already running" << std::endl;
        return false;
    }
    
    status = ServiceStatus::STARTING;
    std::cout << "Starting service " << name << std::endl;
    status = ServiceStatus::RUNNING;
    return true;
}

bool Service::stop() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (status == ServiceStatus::STOPPED) {
        std::cerr << "Service " << name << " is already stopped" << std::endl;
        return false;
    }
    
    status = ServiceStatus::STOPPING;
    std::cout << "Stopping service " << name << std::endl;
    status = ServiceStatus::STOPPED;
    return true;
}

bool Service::register_callback(uint32_t command, ServiceCallback callback) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (callbacks.find(command) != callbacks.end()) {
        std::cerr << "Command " << command << " is already registered for service " << name << std::endl;
        return false;
    }
    
    callbacks[command] = callback;
    std::cout << "Registered command " << command << " for service " << name << std::endl;
    return true;
}

int Service::handle_command(uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (status != ServiceStatus::RUNNING) {
        std::cerr << "Service " << name << " is not running" << std::endl;
        return -1;
    }
    
    auto it = callbacks.find(command);
    if (it == callbacks.end()) {
        std::cerr << "Command " << command << " is not registered for service " << name << std::endl;
        return -1;
    }
    
    std::cout << "Handling command " << command << " for service " << name << std::endl;
    return it->second(command, input, input_size, output, output_size);
}

std::string Service::get_name() const {
    return name;
}

ServiceStatus Service::get_status() const {
    std::lock_guard<std::mutex> lock(mutex);
    return status;
}

bool Service::is_running() const {
    std::lock_guard<std::mutex> lock(mutex);
    return status == ServiceStatus::RUNNING;
}

// ServiceManager implementation
ServiceManager::ServiceManager() {
}

ServiceManager::~ServiceManager() {
    stop_all_services();
}

bool ServiceManager::initialize() {
    std::cout << "Initializing service manager" << std::endl;
    return true;
}

bool ServiceManager::register_service(std::shared_ptr<Service> service) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (services.find(service->get_name()) != services.end()) {
        std::cerr << "Service " << service->get_name() << " is already registered" << std::endl;
        return false;
    }
    
    services[service->get_name()] = service;
    std::cout << "Registered service " << service->get_name() << std::endl;
    return true;
}

bool ServiceManager::unregister_service(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = services.find(name);
    if (it == services.end()) {
        std::cerr << "Service " << name << " is not registered" << std::endl;
        return false;
    }
    
    if (it->second->is_running()) {
        it->second->stop();
    }
    
    services.erase(it);
    std::cout << "Unregistered service " << name << std::endl;
    return true;
}

std::shared_ptr<Service> ServiceManager::get_service(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = services.find(name);
    if (it == services.end()) {
        return nullptr;
    }
    
    return it->second;
}

std::vector<std::shared_ptr<Service>> ServiceManager::get_all_services() {
    std::lock_guard<std::mutex> lock(mutex);
    
    std::vector<std::shared_ptr<Service>> result;
    for (auto &pair : services) {
        result.push_back(pair.second);
    }
    
    return result;
}

bool ServiceManager::start_all_services() {
    std::lock_guard<std::mutex> lock(mutex);
    
    bool success = true;
    for (auto &pair : services) {
        if (!pair.second->start()) {
            std::cerr << "Failed to start service " << pair.first << std::endl;
            success = false;
        }
    }
    
    return success;
}

bool ServiceManager::stop_all_services() {
    std::lock_guard<std::mutex> lock(mutex);
    
    bool success = true;
    for (auto &pair : services) {
        if (!pair.second->stop()) {
            std::cerr << "Failed to stop service " << pair.first << std::endl;
            success = false;
        }
    }
    
    return success;
}

// SystemService implementation
SystemService::SystemService(const std::string &name)
    : Service(name) {
}

SystemService::~SystemService() {
}

// NetworkService implementation
NetworkService::NetworkService()
    : SystemService("NetworkService") {
}

NetworkService::~NetworkService() {
}

bool NetworkService::initialize() {
    std::cout << "Initializing network service" << std::endl;
    
    // Register command callbacks
    register_callback(1, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Network service: Initialize network" << std::endl;
        return 0;
    });
    
    register_callback(2, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Network service: Connect to network" << std::endl;
        return 0;
    });
    
    register_callback(3, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Network service: Disconnect from network" << std::endl;
        return 0;
    });
    
    return true;
}

bool NetworkService::start() {
    if (!SystemService::start()) {
        return false;
    }
    
    std::cout << "Started network service" << std::endl;
    return true;
}

bool NetworkService::stop() {
    if (!SystemService::stop()) {
        return false;
    }
    
    std::cout << "Stopped network service" << std::endl;
    return true;
}

// FileSystemService implementation
FileSystemService::FileSystemService()
    : SystemService("FileSystemService") {
}

FileSystemService::~FileSystemService() {
}

bool FileSystemService::initialize() {
    std::cout << "Initializing file system service" << std::endl;
    
    // Register command callbacks
    register_callback(1, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "File system service: Open file" << std::endl;
        return 0;
    });
    
    register_callback(2, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "File system service: Close file" << std::endl;
        return 0;
    });
    
    register_callback(3, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "File system service: Read file" << std::endl;
        return 0;
    });
    
    register_callback(4, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "File system service: Write file" << std::endl;
        return 0;
    });
    
    return true;
}

bool FileSystemService::start() {
    if (!SystemService::start()) {
        return false;
    }
    
    std::cout << "Started file system service" << std::endl;
    return true;
}

bool FileSystemService::stop() {
    if (!SystemService::stop()) {
        return false;
    }
    
    std::cout << "Stopped file system service" << std::endl;
    return true;
}

// AudioService implementation
AudioService::AudioService()
    : SystemService("AudioService") {
}

AudioService::~AudioService() {
}

bool AudioService::initialize() {
    std::cout << "Initializing audio service" << std::endl;
    
    // Register command callbacks
    register_callback(1, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Audio service: Initialize audio" << std::endl;
        return 0;
    });
    
    register_callback(2, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Audio service: Play audio" << std::endl;
        return 0;
    });
    
    register_callback(3, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Audio service: Stop audio" << std::endl;
        return 0;
    });
    
    return true;
}

bool AudioService::start() {
    if (!SystemService::start()) {
        return false;
    }
    
    std::cout << "Started audio service" << std::endl;
    return true;
}

bool AudioService::stop() {
    if (!SystemService::stop()) {
        return false;
    }
    
    std::cout << "Stopped audio service" << std::endl;
    return true;
}

// InputService implementation
InputService::InputService()
    : SystemService("InputService") {
}

InputService::~InputService() {
}

bool InputService::initialize() {
    std::cout << "Initializing input service" << std::endl;
    
    // Register command callbacks
    register_callback(1, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Input service: Initialize input" << std::endl;
        return 0;
    });
    
    register_callback(2, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Input service: Get button state" << std::endl;
        return 0;
    });
    
    register_callback(3, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Input service: Get touch state" << std::endl;
        return 0;
    });
    
    return true;
}

bool InputService::start() {
    if (!SystemService::start()) {
        return false;
    }
    
    std::cout << "Started input service" << std::endl;
    return true;
}

bool InputService::stop() {
    if (!SystemService::stop()) {
        return false;
    }
    
    std::cout << "Stopped input service" << std::endl;
    return true;
}

// DisplayService implementation
DisplayService::DisplayService()
    : SystemService("DisplayService") {
}

DisplayService::~DisplayService() {
}

bool DisplayService::initialize() {
    std::cout << "Initializing display service" << std::endl;
    
    // Register command callbacks
    register_callback(1, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Display service: Initialize display" << std::endl;
        return 0;
    });
    
    register_callback(2, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Display service: Set display parameters" << std::endl;
        return 0;
    });
    
    register_callback(3, [](uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size) {
        std::cout << "Display service: Get display parameters" << std::endl;
        return 0;
    });
    
    return true;
}

bool DisplayService::start() {
    if (!SystemService::start()) {
        return false;
    }
    
    std::cout << "Started display service" << std::endl;
    return true;
}

bool DisplayService::stop() {
    if (!SystemService::stop()) {
        return false;
    }
    
    std::cout << "Stopped display service" << std::endl;
    return true;
}

} // namespace firmware
