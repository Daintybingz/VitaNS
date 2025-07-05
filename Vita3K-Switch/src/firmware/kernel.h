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
class Process;
class Thread;
class KernelObject;

// Kernel object types
enum class KernelObjectType {
    UNKNOWN,
    PROCESS,
    THREAD,
    SEMAPHORE,
    MUTEX,
    EVENT_FLAG,
    MESSAGE_PIPE,
    CALLBACK,
    TIMER,
    MEMORY_BLOCK
};

// Process status
enum class ProcessStatus {
    CREATED,
    RUNNING,
    WAITING,
    STOPPED,
    EXITED
};

// Thread status
enum class ThreadStatus {
    CREATED,
    RUNNING,
    WAITING,
    READY,
    SUSPENDED,
    STOPPED,
    EXITED
};

// System call handler type
using SyscallHandler = std::function<int(uint32_t, uint32_t, uint32_t, uint32_t)>;

// Kernel class
class Kernel {
public:
    Kernel();
    ~Kernel();

    // Initialize the kernel
    bool initialize();

    // Register a system call handler
    bool register_syscall(uint32_t syscall_id, SyscallHandler handler);

    // Handle a system call
    int handle_syscall(uint32_t syscall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

    // Create a process
    std::shared_ptr<Process> create_process(const std::string &name);

    // Get a process by ID
    std::shared_ptr<Process> get_process(uint32_t process_id);

    // Get all processes
    std::vector<std::shared_ptr<Process>> get_all_processes();

    // Create a kernel object
    template <typename T, typename... Args>
    std::shared_ptr<T> create_object(Args&&... args) {
        auto obj = std::make_shared<T>(std::forward<Args>(args)...);
        register_object(obj);
        return obj;
    }

    // Get a kernel object by ID
    std::shared_ptr<KernelObject> get_object(uint32_t object_id);

    // Register a kernel object
    bool register_object(std::shared_ptr<KernelObject> object);

    // Unregister a kernel object
    bool unregister_object(uint32_t object_id);

private:
    // System call handlers
    std::map<uint32_t, SyscallHandler> syscall_handlers;

    // Processes
    std::map<uint32_t, std::shared_ptr<Process>> processes;

    // Kernel objects
    std::map<uint32_t, std::shared_ptr<KernelObject>> objects;

    // Next process ID
    uint32_t next_process_id;

    // Next object ID
    uint32_t next_object_id;

    // Mutex for thread safety
    std::mutex mutex;
};

// Kernel object base class
class KernelObject {
public:
    KernelObject(KernelObjectType type);
    virtual ~KernelObject();

    // Get object ID
    uint32_t get_id() const;

    // Set object ID
    void set_id(uint32_t id);

    // Get object type
    KernelObjectType get_type() const;

    // Get object name
    std::string get_name() const;

    // Set object name
    void set_name(const std::string &name);

protected:
    // Object ID
    uint32_t id;

    // Object type
    KernelObjectType type;

    // Object name
    std::string name;
};

// Process class
class Process : public KernelObject, public std::enable_shared_from_this<Process> {
public:
    Process(const std::string &name);
    ~Process();

    // Start the process
    bool start();

    // Stop the process
    bool stop();

    // Get process status
    ProcessStatus get_status() const;

    // Create a thread
    std::shared_ptr<Thread> create_thread(const std::string &name);

    // Get a thread by ID
    std::shared_ptr<Thread> get_thread(uint32_t thread_id);

    // Get all threads
    std::vector<std::shared_ptr<Thread>> get_all_threads();

private:
    // Process status
    ProcessStatus status;

    // Threads
    std::map<uint32_t, std::shared_ptr<Thread>> threads;

    // Next thread ID
    uint32_t next_thread_id;

    // Mutex for thread safety
    std::mutex mutex;
};

// Thread class
class Thread : public KernelObject {
public:
    Thread(const std::string &name, std::shared_ptr<Process> process);
    ~Thread();

    // Start the thread
    bool start();

    // Stop the thread
    bool stop();

    // Suspend the thread
    bool suspend();

    // Resume the thread
    bool resume();

    // Get thread status
    ThreadStatus get_status() const;

    // Get parent process
    std::shared_ptr<Process> get_process() const;

private:
    // Thread status
    ThreadStatus status;

    // Parent process
    std::weak_ptr<Process> process;
};

// Semaphore class
class Semaphore : public KernelObject {
public:
    Semaphore(const std::string &name, int initial_count, int max_count);
    ~Semaphore();

    // Signal the semaphore
    bool signal(int count = 1);

    // Wait for the semaphore
    bool wait(uint32_t timeout_us = 0);

    // Try to acquire the semaphore
    bool try_wait();

    // Get current count
    int get_count() const;

private:
    // Current count
    int count;

    // Maximum count
    int max_count;

    // Mutex for thread safety
    mutable std::mutex mutex;
};

// Mutex class
class Mutex : public KernelObject {
public:
    Mutex(const std::string &name);
    ~Mutex();

    // Lock the mutex
    bool lock(uint32_t timeout_us = 0);

    // Try to lock the mutex
    bool try_lock();

    // Unlock the mutex
    bool unlock();

    // Check if locked
    bool is_locked() const;

private:
    // Locked flag
    bool locked;

    // Owner thread ID
    uint32_t owner_thread_id;

    // Mutex for thread safety
    mutable std::mutex mutex;
};

// Event flag class
class EventFlag : public KernelObject {
public:
    EventFlag(const std::string &name, uint32_t initial_pattern);
    ~EventFlag();

    // Set bits in the pattern
    bool set(uint32_t pattern);

    // Clear bits in the pattern
    bool clear(uint32_t pattern);

    // Wait for bits in the pattern
    bool wait(uint32_t pattern, uint32_t wait_mode, uint32_t *result_pattern, uint32_t timeout_us = 0);

    // Try to wait for bits in the pattern
    bool try_wait(uint32_t pattern, uint32_t wait_mode, uint32_t *result_pattern);

    // Get current pattern
    uint32_t get_pattern() const;

private:
    // Current pattern
    uint32_t pattern;

    // Mutex for thread safety
    mutable std::mutex mutex;
};

} // namespace firmware
