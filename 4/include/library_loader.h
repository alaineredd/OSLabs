#ifndef LIBRARY_LOADER_H
#define LIBRARY_LOADER_H

#include <string>
#include <iostream>
#include <memory>

#ifdef _WIN32
    #include <windows.h>
    #define LIB_PREFIX ""
    #define LIB_SUFFIX ".dll"
    #define LIB_HANDLE HMODULE
    #define LIB_NULL nullptr
#else
    #include <dlfcn.h>
    #define LIB_PREFIX "lib"
    #define LIB_SUFFIX ".so"
    #define LIB_HANDLE void*
    #define LIB_NULL nullptr
#endif

class LibraryLoader {
private:
    LIB_HANDLE handle_;
    std::string current_lib_path_;
    std::string lib_name_;

public:
    LibraryLoader() : handle_(LIB_NULL) {}
    
    ~LibraryLoader() {
        unload();
    }
    
    bool load(const std::string& libraryName) {
        unload();
        
        lib_name_ = libraryName;
        std::string full_path = LIB_PREFIX + libraryName + LIB_SUFFIX;
        current_lib_path_ = full_path;
        
        std::cout << "Загружаем библиотеку: " << full_path << std::endl;
        
#ifdef _WIN32
        // Windows
        handle_ = LoadLibraryA(full_path.c_str());
        if (!handle_) {
            std::string with_path = ".\\" + full_path;
            handle_ = LoadLibraryA(with_path.c_str());
            if (handle_) {
                current_lib_path_ = with_path;
            }
        }
#else
        // Linux
        handle_ = dlopen(full_path.c_str(), RTLD_LAZY);
        if (!handle_) {
            std::string with_path = "./" + full_path;
            handle_ = dlopen(with_path.c_str(), RTLD_LAZY);
            if (handle_) {
                current_lib_path_ = with_path;
            }
        }
        
        if (!handle_) {
            std::string no_prefix = libraryName + LIB_SUFFIX;
            handle_ = dlopen(no_prefix.c_str(), RTLD_LAZY);
            if (handle_) {
                current_lib_path_ = no_prefix;
            }
        }
#endif
        
        if (!handle_) {
            std::cerr << "Ошибка загрузки библиотеки '" << libraryName 
                     << "': " << getError() << std::endl;
            return false;
        }
        
        std::cout << "Библиотека успешно загружена: " << current_lib_path_ << std::endl;
        return true;
    }
    
    void unload() {
        if (handle_ != LIB_NULL) {
#ifdef _WIN32
            FreeLibrary(handle_);
#else
            dlclose(handle_);
#endif
            handle_ = LIB_NULL;
            current_lib_path_.clear();
            lib_name_.clear();
        }
    }
    
    template<typename T>
    T getFunction(const std::string& functionName) {
        if (handle_ == LIB_NULL) {
            std::cerr << "Ошибка: библиотека не загружена!" << std::endl;
            return nullptr;
        }
        
#ifdef _WIN32
        FARPROC func = GetProcAddress(handle_, functionName.c_str());
#else
        void* func = dlsym(handle_, functionName.c_str());
#endif
        
        if (!func) {
            std::cerr << "Функция '" << functionName << "' не найдена в библиотеке '"
                     << lib_name_ << "'!" << std::endl;
            std::cerr << "Детали ошибки: " << getError() << std::endl;
            return nullptr;
        }
        
        return reinterpret_cast<T>(func);
    }
    
    std::string getError() const {
#ifdef _WIN32
        DWORD error = GetLastError();
        if (error == 0) return "Нет ошибки";
        
        char* msgBuffer = nullptr;
        size_t size = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, 
            error, 
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&msgBuffer, 
            0, 
            NULL);
        
        if (size == 0) {
            return "Не удалось получить описание ошибки";
        }
        
        std::string message(msgBuffer, size);
        LocalFree(msgBuffer);
        return message;
#else
        const char* error = dlerror();
        return error ? std::string(error) : "Нет ошибки";
#endif
    }
    
    std::string getCurrentLibraryPath() const {
        return current_lib_path_;
    }
    
    std::string getLibraryName() const {
        return lib_name_;
    }
    
    bool isLoaded() const {
        return handle_ != LIB_NULL;
    }
    
    LibraryLoader(const LibraryLoader&) = delete;
    LibraryLoader& operator=(const LibraryLoader&) = delete;
    
    LibraryLoader(LibraryLoader&& other) noexcept 
        : handle_(other.handle_)
        , current_lib_path_(std::move(other.current_lib_path_))
        , lib_name_(std::move(other.lib_name_)) {
        other.handle_ = LIB_NULL;
    }
    
    LibraryLoader& operator=(LibraryLoader&& other) noexcept {
        if (this != &other) {
            unload();
            handle_ = other.handle_;
            current_lib_path_ = std::move(other.current_lib_path_);
            lib_name_ = std::move(other.lib_name_);
            other.handle_ = LIB_NULL;
        }
        return *this;
    }
};

#endif // LIBRARY_LOADER_H