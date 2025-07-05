#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <mutex>

namespace firmware {

// Forward declarations
class Service;
class ServiceManager;

// Service callback type
using ServiceCallback = std::function<int(uint32_t, void*, uint32_t, void*, uint32_t)>;

// Service status
enum class ServiceStatus {
    STOPPED,
    STARTING,
    RUNNING,
    STOPPING
};

// Service class
class Service {
public:
    Service(const std::string &name);
    virtual ~Service();

    // Start the service
    virtual bool start();

    // Stop the service
    virtual bool stop();

    // Register a callback for a specific command
    virtual bool register_callback(uint32_t command, ServiceCallback callback);

    // Handle a command
    virtual int handle_command(uint32_t command, void *input, uint32_t input_size, void *output, uint32_t output_size);

    // Get service name
    std::string get_name() const;

    // Get service status
    ServiceStatus get_status() const;

    // Check if service is running
    bool is_running() const;

protected:
    // Service name
    std::string name;

    // Service status
    ServiceStatus status;

    // Command callbacks
    std::map<uint32_t, ServiceCallback> callbacks;

    // Mutex for thread safety
    mutable std::mutex mutex;
};

// Service manager class
class ServiceManager {
public:
    ServiceManager();
    ~ServiceManager();

    // Initialize the service manager
    bool initialize();

    // Register a service
    bool register_service(std::shared_ptr<Service> service);

    // Unregister a service
    bool unregister_service(const std::string &name);

    // Get a service by name
    std::shared_ptr<Service> get_service(const std::string &name);

    // Get all services
    std::vector<std::shared_ptr<Service>> get_all_services();

    // Start all services
    bool start_all_services();

    // Stop all services
    bool stop_all_services();

private:
    // Services
    std::map<std::string, std::shared_ptr<Service>> services;

    // Mutex for thread safety
    std::mutex mutex;
};

// System service base class
class SystemService : public Service {
public:
    SystemService(const std::string &name);
    virtual ~SystemService();

    // Initialize the service
    virtual bool initialize() = 0;
};

// Network service class
class NetworkService : public SystemService {
public:
    NetworkService();
    ~NetworkService();

    // Initialize the service
    bool initialize() override;

    // Start the service
    bool start() override;

    // Stop the service
    bool stop() override;

private:
    // Network service implementation
    // This will be implemented in a future update
};

// File system service class
class FileSystemService : public SystemService {
public:
    FileSystemService();
    ~FileSystemService();

    // Initialize the service
    bool initialize() override;

    // Start the service
    bool start() override;

    // Stop the service
    bool stop() override;

private:
    // File system service implementation
    // This will be implemented in a future update
};

// Audio service class
class AudioService : public SystemService {
public:
    AudioService();
    ~AudioService();

    // Initialize the service
    bool initialize() override;

    // Start the service
    bool start() override;

    // Stop the service
    bool stop() override;

private:
    // Audio service implementation
    // This will be implemented in a future update
};

// Input service class
class InputService : public SystemService {
public:
    InputService();
    ~InputService();

    // Initialize the service
    bool initialize() override;

    // Start the service
    bool start() override;

    // Stop the service
    bool stop() override;

private:
    // Input service implementation
    // This will be implemented in a future update
};

// Display service class
class DisplayService : public SystemService {
public:
    DisplayService();
    ~DisplayService();

    // Initialize the service
    bool initialize() override;

    // Start the service
    bool start() override;

    // Stop the service
    bool stop() override;

private:
    // Display service implementation
    // This will be implemented in a future update
};

} // namespace firmware
