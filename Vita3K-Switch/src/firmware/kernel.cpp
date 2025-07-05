#include "kernel.h"
#include <iostream>
#include <algorithm>

namespace firmware {

// KernelObject implementation
KernelObject::KernelObject(KernelObjectType type)
    : id(0), type(type) {
}

KernelObject::~KernelObject() {
}

uint32_t KernelObject::get_id() const {
    return id;
}

void KernelObject::set_id(uint32_t id) {
    this->id = id;
}

KernelObjectType KernelObject::get_type() const {
    return type;
}

std::string KernelObject::get_name() const {
    return name;
}

void KernelObject::set_name(const std::string &name) {
    this->name = name;
}

// Process implementation
Process::Process(const std::string &name)
    : KernelObject(KernelObjectType::PROCESS), status(ProcessStatus::CREATED), next_thread_id(1) {
    set_name(name);
}

Process::~Process() {
    // Stop all threads
    for (auto &pair : threads) {
        pair.second->stop();
    }
    threads.clear();
}

bool Process::start() {
    if (status != ProcessStatus::CREATED && status != ProcessStatus::STOPPED) {
        std::cerr << "Process " << get_name() << " is not in a startable state" << std::endl;
        return false;
    }

    std::cout << "Starting process " << get_name() << std::endl;
    status = ProcessStatus::RUNNING;
    return true;
}

bool Process::stop() {
    if (status != ProcessStatus::RUNNING && status != ProcessStatus::WAITING) {
        std::cerr << "Process " << get_name() << " is not in a stoppable state" << std::endl;
        return false;
    }

    // Stop all threads
    for (auto &pair : threads) {
        pair.second->stop();
    }

    std::cout << "Stopping process " << get_name() << std::endl;
    status = ProcessStatus::STOPPED;
    return true;
}

ProcessStatus Process::get_status() const {
    return status;
}

std::shared_ptr<Thread> Process::create_thread(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex);

    // Create thread
    auto thread = std::make_shared<Thread>(name, shared_from_this());
    
    // Assign thread ID
    uint32_t thread_id = next_thread_id++;
    thread->set_id(thread_id);
    
    // Add to threads map
    threads[thread_id] = thread;
    
    std::cout << "Created thread " << name << " (ID: " << thread_id << ") in process " << get_name() << std::endl;
    return thread;
}

std::shared_ptr<Thread> Process::get_thread(uint32_t thread_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = threads.find(thread_id);
    if (it == threads.end()) {
        return nullptr;
    }
    
    return it->second;
}

std::vector<std::shared_ptr<Thread>> Process::get_all_threads() {
    std::lock_guard<std::mutex> lock(mutex);
    
    std::vector<std::shared_ptr<Thread>> result;
    for (auto &pair : threads) {
        result.push_back(pair.second);
    }
    
    return result;
}

// Thread implementation
Thread::Thread(const std::string &name, std::shared_ptr<Process> process)
    : KernelObject(KernelObjectType::THREAD), status(ThreadStatus::CREATED), process(process) {
    set_name(name);
}

Thread::~Thread() {
    if (status == ThreadStatus::RUNNING || status == ThreadStatus::READY || status == ThreadStatus::WAITING) {
        stop();
    }
}

bool Thread::start() {
    if (status != ThreadStatus::CREATED && status != ThreadStatus::STOPPED && status != ThreadStatus::SUSPENDED) {
        std::cerr << "Thread " << get_name() << " is not in a startable state" << std::endl;
        return false;
    }

    std::cout << "Starting thread " << get_name() << std::endl;
    status = ThreadStatus::RUNNING;
    return true;
}

bool Thread::stop() {
    if (status == ThreadStatus::STOPPED || status == ThreadStatus::EXITED) {
        std::cerr << "Thread " << get_name() << " is already stopped" << std::endl;
        return false;
    }

    std::cout << "Stopping thread " << get_name() << std::endl;
    status = ThreadStatus::STOPPED;
    return true;
}

bool Thread::suspend() {
    if (status != ThreadStatus::RUNNING && status != ThreadStatus::READY) {
        std::cerr << "Thread " << get_name() << " is not in a suspendable state" << std::endl;
        return false;
    }

    std::cout << "Suspending thread " << get_name() << std::endl;
    status = ThreadStatus::SUSPENDED;
    return true;
}

bool Thread::resume() {
    if (status != ThreadStatus::SUSPENDED) {
        std::cerr << "Thread " << get_name() << " is not suspended" << std::endl;
        return false;
    }

    std::cout << "Resuming thread " << get_name() << std::endl;
    status = ThreadStatus::READY;
    return true;
}

ThreadStatus Thread::get_status() const {
    return status;
}

std::shared_ptr<Process> Thread::get_process() const {
    return process.lock();
}

// Semaphore implementation
Semaphore::Semaphore(const std::string &name, int initial_count, int max_count)
    : KernelObject(KernelObjectType::SEMAPHORE), count(initial_count), max_count(max_count) {
    set_name(name);
}

Semaphore::~Semaphore() {
}

bool Semaphore::signal(int count) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (count <= 0) {
        std::cerr << "Invalid count for semaphore signal: " << count << std::endl;
        return false;
    }
    
    if (this->count + count > max_count) {
        std::cerr << "Semaphore count would exceed max count" << std::endl;
        return false;
    }
    
    this->count += count;
    std::cout << "Signaled semaphore " << get_name() << " (count: " << this->count << ")" << std::endl;
    return true;
}

bool Semaphore::wait(uint32_t timeout_us) {
    // In a real implementation, this would block until the semaphore is signaled
    // or the timeout expires. For now, just check if we can acquire the semaphore.
    std::lock_guard<std::mutex> lock(mutex);
    
    if (count <= 0) {
        std::cerr << "Semaphore " << get_name() << " count is 0, cannot wait" << std::endl;
        return false;
    }
    
    count--;
    std::cout << "Waited on semaphore " << get_name() << " (count: " << count << ")" << std::endl;
    return true;
}

bool Semaphore::try_wait() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (count <= 0) {
        return false;
    }
    
    count--;
    std::cout << "Try-waited on semaphore " << get_name() << " (count: " << count << ")" << std::endl;
    return true;
}

int Semaphore::get_count() const {
    std::lock_guard<std::mutex> lock(mutex);
    return count;
}

// Mutex implementation
Mutex::Mutex(const std::string &name)
    : KernelObject(KernelObjectType::MUTEX), locked(false), owner_thread_id(0) {
    set_name(name);
}

Mutex::~Mutex() {
}

bool Mutex::lock(uint32_t timeout_us) {
    // In a real implementation, this would block until the mutex is unlocked
    // or the timeout expires. For now, just check if we can acquire the mutex.
    std::lock_guard<std::mutex> lock(mutex);
    
    if (locked) {
        std::cerr << "Mutex " << get_name() << " is already locked" << std::endl;
        return false;
    }
    
    locked = true;
    owner_thread_id = 1; // Dummy thread ID for now
    std::cout << "Locked mutex " << get_name() << std::endl;
    return true;
}

bool Mutex::try_lock() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (locked) {
        return false;
    }
    
    locked = true;
    owner_thread_id = 1; // Dummy thread ID for now
    std::cout << "Try-locked mutex " << get_name() << std::endl;
    return true;
}

bool Mutex::unlock() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!locked) {
        std::cerr << "Mutex " << get_name() << " is not locked" << std::endl;
        return false;
    }
    
    locked = false;
    owner_thread_id = 0;
    std::cout << "Unlocked mutex " << get_name() << std::endl;
    return true;
}

bool Mutex::is_locked() const {
    std::lock_guard<std::mutex> lock(mutex);
    return locked;
}

// EventFlag implementation
EventFlag::EventFlag(const std::string &name, uint32_t initial_pattern)
    : KernelObject(KernelObjectType::EVENT_FLAG), pattern(initial_pattern) {
    set_name(name);
}

EventFlag::~EventFlag() {
}

bool EventFlag::set(uint32_t pattern) {
    std::lock_guard<std::mutex> lock(mutex);
    
    this->pattern |= pattern;
    std::cout << "Set event flag " << get_name() << " pattern: 0x" << std::hex << this->pattern << std::dec << std::endl;
    return true;
}

bool EventFlag::clear(uint32_t pattern) {
    std::lock_guard<std::mutex> lock(mutex);
    
    this->pattern &= ~pattern;
    std::cout << "Cleared event flag " << get_name() << " pattern: 0x" << std::hex << this->pattern << std::dec << std::endl;
    return true;
}

bool EventFlag::wait(uint32_t pattern, uint32_t wait_mode, uint32_t *result_pattern, uint32_t timeout_us) {
    // In a real implementation, this would block until the pattern is matched
    // or the timeout expires. For now, just check if the pattern matches.
    std::lock_guard<std::mutex> lock(mutex);
    
    // Wait modes:
    // 0: Wait for any bit in the pattern to be set
    // 1: Wait for all bits in the pattern to be set
    // 2: Wait for any bit in the pattern to be set, then clear the matched bits
    // 3: Wait for all bits in the pattern to be set, then clear the matched bits
    
    bool match = false;
    if (wait_mode & 1) {
        // Wait for all bits
        match = (this->pattern & pattern) == pattern;
    } else {
        // Wait for any bit
        match = (this->pattern & pattern) != 0;
    }
    
    if (!match) {
        std::cerr << "Event flag " << get_name() << " pattern does not match" << std::endl;
        return false;
    }
    
    if (result_pattern) {
        *result_pattern = this->pattern;
    }
    
    if (wait_mode & 2) {
        // Clear matched bits
        if (wait_mode & 1) {
            // Clear all matched bits
            this->pattern &= ~pattern;
        } else {
            // Clear any matched bits
            this->pattern &= ~(this->pattern & pattern);
        }
    }
    
    std::cout << "Waited on event flag " << get_name() << " pattern: 0x" << std::hex << this->pattern << std::dec << std::endl;
    return true;
}

bool EventFlag::try_wait(uint32_t pattern, uint32_t wait_mode, uint32_t *result_pattern) {
    return wait(pattern, wait_mode, result_pattern, 0);
}

uint32_t EventFlag::get_pattern() const {
    std::lock_guard<std::mutex> lock(mutex);
    return pattern;
}

// Kernel implementation
Kernel::Kernel()
    : next_process_id(1), next_object_id(1) {
}

Kernel::~Kernel() {
    // Unregister all objects
    objects.clear();
    
    // Stop all processes
    for (auto &pair : processes) {
        pair.second->stop();
    }
    processes.clear();
}

bool Kernel::initialize() {
    std::cout << "Initializing kernel" << std::endl;
    
    // Register system call handlers
    // This will be implemented in a future update
    
    return true;
}

bool Kernel::register_syscall(uint32_t syscall_id, SyscallHandler handler) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (syscall_handlers.find(syscall_id) != syscall_handlers.end()) {
        std::cerr << "System call " << syscall_id << " is already registered" << std::endl;
        return false;
    }
    
    syscall_handlers[syscall_id] = handler;
    std::cout << "Registered system call " << syscall_id << std::endl;
    return true;
}

int Kernel::handle_syscall(uint32_t syscall_id, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = syscall_handlers.find(syscall_id);
    if (it == syscall_handlers.end()) {
        std::cerr << "System call " << syscall_id << " is not registered" << std::endl;
        return -1;
    }
    
    std::cout << "Handling system call " << syscall_id << std::endl;
    return it->second(arg1, arg2, arg3, arg4);
}

std::shared_ptr<Process> Kernel::create_process(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Create process
    auto process = std::make_shared<Process>(name);
    
    // Assign process ID
    uint32_t process_id = next_process_id++;
    process->set_id(process_id);
    
    // Register process as a kernel object
    register_object(process);
    
    // Add to processes map
    processes[process_id] = process;
    
    std::cout << "Created process " << name << " (ID: " << process_id << ")" << std::endl;
    return process;
}

std::shared_ptr<Process> Kernel::get_process(uint32_t process_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = processes.find(process_id);
    if (it == processes.end()) {
        return nullptr;
    }
    
    return it->second;
}

std::vector<std::shared_ptr<Process>> Kernel::get_all_processes() {
    std::lock_guard<std::mutex> lock(mutex);
    
    std::vector<std::shared_ptr<Process>> result;
    for (auto &pair : processes) {
        result.push_back(pair.second);
    }
    
    return result;
}

std::shared_ptr<KernelObject> Kernel::get_object(uint32_t object_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = objects.find(object_id);
    if (it == objects.end()) {
        return nullptr;
    }
    
    return it->second;
}

bool Kernel::register_object(std::shared_ptr<KernelObject> object) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (object->get_id() == 0) {
        // Assign object ID
        uint32_t object_id = next_object_id++;
        object->set_id(object_id);
    }
    
    objects[object->get_id()] = object;
    std::cout << "Registered kernel object " << object->get_name() << " (ID: " << object->get_id() << ")" << std::endl;
    return true;
}

bool Kernel::unregister_object(uint32_t object_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = objects.find(object_id);
    if (it == objects.end()) {
        std::cerr << "Kernel object " << object_id << " is not registered" << std::endl;
        return false;
    }
    
    std::cout << "Unregistered kernel object " << it->second->get_name() << " (ID: " << object_id << ")" << std::endl;
    objects.erase(it);
    return true;
}

} // namespace firmware
