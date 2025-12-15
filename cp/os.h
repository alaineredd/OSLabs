#pragma once

#include <string>
#include <memory>

namespace OSSignals {
    const int SIG_INT = 2;
    const int SIG_TERM = 15;
}


using SignalHandler = void(*)(int);

class OSInterface {
public:
    virtual ~OSInterface() = default;
    
    virtual bool createSharedMemory(const std::string& name, size_t size) = 0;
    virtual bool openSharedMemory(const std::string& name) = 0;
    virtual char* mapSharedMemory(size_t size) = 0;
    virtual void unmapSharedMemory() = 0;
    virtual void closeSharedMemory() = 0;
    virtual void destroySharedMemory(const std::string& name) = 0;
    
    virtual bool createSemaphore(const std::string& name) = 0;
    virtual bool openSemaphore(const std::string& name) = 0;
    virtual void waitSemaphore() = 0;
    virtual void postSemaphore() = 0;
    virtual void closeSemaphore() = 0;
    virtual void destroySemaphore(const std::string& name) = 0;
    
    virtual void* createThread(void* (*start_routine)(void*), void* arg) = 0;
    virtual void joinThread(void* thread) = 0;
    virtual void detachThread(void* thread) = 0;
    
    virtual void sleepMilliseconds(unsigned int ms) = 0;
    virtual void sleepMicroseconds(unsigned int us) = 0;
    
    virtual void setupSignalHandler(int signal, SignalHandler handler) = 0;
    virtual void setupInterruptHandler(SignalHandler handler) = 0;

    virtual bool fileExists(const std::string& path) = 0;

    virtual std::string getOSName() = 0;

    virtual void cleanupResources() = 0;
    
protected:
    int shm_fd = -1;
    char* shm_ptr = nullptr;
    void* semaphore = nullptr;
};

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

namespace SharedResources {
    const std::string SHARED_MEMORY_REQUEST = "/bulls_cows_shm_request";
    const std::string SHARED_MEMORY_RESPONSE = "/bulls_cows_shm_response";
    const std::string SEMAPHORE_NAME = "/bulls_cows_sem";
    const size_t DEFAULT_SHM_SIZE = 4096;
}