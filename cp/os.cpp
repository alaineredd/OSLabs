#include "os.h"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <atomic>
#include <cstdlib>

class OSLinux : public OSInterface {
private:
    pthread_t current_thread;
    std::atomic<bool> resources_cleaned{false};
    SignalHandler current_signal_handler = nullptr;
    
public:
    OSLinux() = default;
    ~OSLinux() override {
        cleanupResources();
    }
    
    // Shared Memory
    bool createSharedMemory(const std::string& name, size_t size) override {
        shm_fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            std::cerr << "Failed to create shared memory: " << strerror(errno) << std::endl;
            return false;
        }
        
        if (ftruncate(shm_fd, size) == -1) {
            std::cerr << "Failed to set shared memory size: " << strerror(errno) << std::endl;
            close(shm_fd);
            return false;
        }
        
        return true;
    }
    
    bool openSharedMemory(const std::string& name) override {
        shm_fd = shm_open(name.c_str(), O_RDWR, 0666);
        if (shm_fd == -1) {
            std::cerr << "Failed to open shared memory: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }
    
    char* mapSharedMemory(size_t size) override {
        shm_ptr = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shm_ptr == MAP_FAILED) {
            std::cerr << "Failed to map shared memory: " << strerror(errno) << std::endl;
            shm_ptr = nullptr;
            return nullptr;
        }
        return shm_ptr;
    }
    
    void unmapSharedMemory() override {
        if (shm_ptr != nullptr && shm_ptr != MAP_FAILED) {
            munmap(shm_ptr, SharedResources::DEFAULT_SHM_SIZE);
            shm_ptr = nullptr;
        }
    }
    
    void closeSharedMemory() override {
        if (shm_fd != -1) {
            close(shm_fd);
            shm_fd = -1;
        }
    }
    
    void destroySharedMemory(const std::string& name) override {
        shm_unlink(name.c_str());
    }
    
    // Semaphores
    bool createSemaphore(const std::string& name) override {
        semaphore = sem_open(name.c_str(), O_CREAT, 0644, 1);
        if (semaphore == SEM_FAILED) {
            std::cerr << "Failed to create semaphore: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }
    
    bool openSemaphore(const std::string& name) override {
        semaphore = sem_open(name.c_str(), 0);
        if (semaphore == SEM_FAILED) {
            std::cerr << "Failed to open semaphore: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
    }
    
    void waitSemaphore() override {
        if (semaphore != SEM_FAILED && semaphore != nullptr) {
            sem_wait((sem_t*)semaphore);
        }
    }
    
    void postSemaphore() override {
        if (semaphore != SEM_FAILED && semaphore != nullptr) {
            sem_post((sem_t*)semaphore);
        }
    }
    
    void closeSemaphore() override {
        if (semaphore != SEM_FAILED && semaphore != nullptr) {
            sem_close((sem_t*)semaphore);
            semaphore = nullptr;
        }
    }
    
    void destroySemaphore(const std::string& name) override {
        sem_unlink(name.c_str());
    }
    
    // Threads
    void* createThread(void* (*start_routine)(void*), void* arg) override {
        pthread_t* thread = new pthread_t;
        if (pthread_create(thread, nullptr, start_routine, arg) != 0) {
            delete thread;
            return nullptr;
        }
        return thread;
    }
    
    void joinThread(void* thread) override {
        if (thread != nullptr) {
            pthread_join(*static_cast<pthread_t*>(thread), nullptr);
            delete static_cast<pthread_t*>(thread);
        }
    }
    
    void detachThread(void* thread) override {
        if (thread != nullptr) {
            pthread_detach(*static_cast<pthread_t*>(thread));
        }
    }
    
    // Sleep/Delay
    void sleepMilliseconds(unsigned int ms) override {
        usleep(ms * 1000);
    }
    
    void sleepMicroseconds(unsigned int us) override {
        usleep(us);
    }
    
    // Signals
    static void signalHandlerWrapper(int sig) {
        // Этот обработчик будет установлен через sigaction
        // Реальный обработчик будет вызываться из setupSignalHandler
    }
    
    void setupSignalHandler(int signal, SignalHandler handler) override {
        current_signal_handler = handler;
        
        struct sigaction sa;
        sa.sa_handler = handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        
        if (sigaction(signal, &sa, NULL) == -1) {
            std::cerr << "Failed to setup signal handler: " << strerror(errno) << std::endl;
        }
    }
    
    void setupInterruptHandler(SignalHandler handler) override {
        setupSignalHandler(OSSignals::SIG_INT, handler);
    }
    
    // File operations
    bool fileExists(const std::string& path) override {
        return access(path.c_str(), F_OK) != -1;
    }
    
    // System info
    std::string getOSName() override {
        return "Linux";
    }
    
    // Cleanup
    void cleanupResources() override {
        if (resources_cleaned.exchange(true)) {
            return; // Already cleaned
        }
        
        unmapSharedMemory();
        closeSharedMemory();
        closeSemaphore();
    }
};

// Реализация фабрики
std::unique_ptr<OSInterface> OSFactory::create(OSType type) {
    switch (type) {
        case OS_LINUX:
            return std::make_unique<OSLinux>();
        case OS_WINDOWS:
        case OS_MACOS:
        default:
            std::cerr << "OS type not supported yet" << std::endl;
            return nullptr;
    }
}

std::unique_ptr<OSInterface> OSFactory::createForCurrentOS() {
    OSType type = detectOS();
    return create(type);
}

OSFactory::OSType OSFactory::detectOS() {
    #ifdef __linux__
        return OS_LINUX;
    #elif defined(_WIN32) || defined(_WIN64)
        return OS_WINDOWS;
    #elif defined(__APPLE__)
        return OS_MACOS;
    #else
        return OS_LINUX; // Default to Linux for other Unix-like systems
    #endif
}