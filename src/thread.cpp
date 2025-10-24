#include <stdexcept>
#include <iostream>
#include <cstring>

#include "thread.hpp"

namespace thread{

struct ThreadData
{
    pthread_t thread = 0;
    bool is_running = false;
};

Thread::Thread(ThreadFunc func) : func_(func) {
    data_ = new ThreadData();
}

void Thread::Join() {
    int code = pthread_join(data_->thread, NULL);
    if(code != 0) {
        throw std::runtime_error("thread join error");
    }
}

void Thread::Detach() {
    int code = pthread_detach(data_->thread);
    if(code != 0) {
        throw std::runtime_error("thread detach error");
    }
    data_->is_running = false;
}

Thread::Thread(Thread&& other) noexcept : func_(other.func_), data_(other.data_) {
    other.data_ = nullptr;
    other.func_ = nullptr;
}


Thread& Thread::operator=(Thread&& other) noexcept {
    if (this != &other) {
        if (data_) {
            delete data_;
        }
        func_ = other.func_;
        data_ = other.data_;
        other.func_ = nullptr;
        other.data_ = nullptr;
    }
    return *this;
}

void Thread::Run(void *data) {
    int code = pthread_create(&(data_->thread), NULL, func_, data);
    if(code != 0) {
        throw std::runtime_error("thread create error");
    }
}

Thread::~Thread() {
    if (data_) {
        delete data_;
        data_ = nullptr;
    }
}
} // namespace thread