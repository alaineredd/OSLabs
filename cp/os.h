#pragma once

#include <string>
#include <memory>

// Константы сигналов (кросс-платформенные)
namespace OSSignals {
    const int SIG_INT = 2;  // SIGINT
    const int SIG_TERM = 15; // SIGTERM
}

// Тип для обработчика сигналов
using SignalHandler = void(*)(int);

// Абстрактный класс для OS-специфичных операций
class OSInterface {
public:
    virtual ~OSInterface() = default;
    
    // Shared Memory
    virtual bool createSharedMemory(const std::string& name, size_t size) = 0;
    virtual bool openSharedMemory(const std::string& name) = 0;
    virtual char* mapSharedMemory(size_t size) = 0;
    virtual void unmapSharedMemory() = 0;
    virtual void closeSharedMemory() = 0;
    virtual void destroySharedMemory(const std::string& name) = 0;
    
    // Semaphores
    virtual bool createSemaphore(const std::string& name) = 0;
    virtual bool openSemaphore(const std::string& name) = 0;
    virtual void waitSemaphore() = 0;
    virtual void postSemaphore() = 0;
    virtual void closeSemaphore() = 0;
    virtual void destroySemaphore(const std::string& name) = 0;
    
    // Threads
    virtual void* createThread(void* (*start_routine)(void*), void* arg) = 0;
    virtual void joinThread(void* thread) = 0;
    virtual void detachThread(void* thread) = 0;
    
    // Sleep/Delay
    virtual void sleepMilliseconds(unsigned int ms) = 0;
    virtual void sleepMicroseconds(unsigned int us) = 0;
    
    // Signals
    virtual void setupSignalHandler(int signal, SignalHandler handler) = 0;
    virtual void setupInterruptHandler(SignalHandler handler) = 0;
    
    // File operations
    virtual bool fileExists(const std::string& path) = 0;
    
    // System info
    virtual std::string getOSName() = 0;
    
    // Cleanup
    virtual void cleanupResources() = 0;
    
protected:
    int shm_fd = -1;
    char* shm_ptr = nullptr;
    void* semaphore = nullptr;
};

// Фабрика для создания OS-специфичной реализации
class OSFactory {
public:
    enum OSType {
        OS_LINUX,
        OS_WINDOWS,
        OS_MACOS
    };
    
    static std::unique_ptr<OSInterface> create(OSType type);
    static std::unique_ptr<OSInterface> createForCurrentOS();
    
private:
    static OSType detectOS();
};

// Константы для имен ресурсов
namespace SharedResources {
    const std::string SHARED_MEMORY_REQUEST = "/bulls_cows_shm_request";
    const std::string SHARED_MEMORY_RESPONSE = "/bulls_cows_shm_response";
    const std::string SEMAPHORE_NAME = "/bulls_cows_sem";
    const size_t DEFAULT_SHM_SIZE = 4096;
}