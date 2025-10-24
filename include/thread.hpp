#pragma once

#include <pthread.h>

namespace thread{
struct ThreadData;
using ThreadFunc = typeof(void*(void*))*;
class Thread {
public:

    Thread(ThreadFunc func);

    Thread(const Thread&) = delete;

    Thread& operator=(const Thread&) = delete;

    Thread(Thread&& other) noexcept;

    Thread& operator=(Thread&& other) noexcept;

    void Join();

    void Detach();

    void Run(void *);

    ~Thread();
private:
    ThreadFunc func_;
    ThreadData* data_;
};
} // namespace thread