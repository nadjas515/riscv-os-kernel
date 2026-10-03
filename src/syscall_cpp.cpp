//
// Created by os on 6/20/26.
//
#include "../inc/syscall_cpp.hpp"

#include "../inc/Scheduler.hpp"

void* operator new(size_t size) {
    return mem_alloc(size);
}

void operator delete(void* ptr) noexcept{
    mem_free(ptr);
}

Thread::Thread (void (*body)(void*), void* arg) {
    this->body = body;
    this->arg = arg;
    myHandle=0;
}

Thread::Thread() {
    body=runWrapper;
    arg=this;
    myHandle=0;
}

int Thread::start() {
    if (!myHandle)
        return thread_create(&myHandle,body,arg);
    return 0;
}

void Thread::dispatch() {
    thread_dispatch();
}

int Thread::sleep(time_t t) {
    return time_sleep(t);
}

void Thread::runWrapper(void* arg) {
    ((Thread*)arg)->run();
}

Thread::~Thread() {}

Semaphore::Semaphore(unsigned init) {
    sem_open(&myHandle,init);
}

int Semaphore::signal() {
    return sem_signal(myHandle);
}

int Semaphore::wait() {
    return sem_wait(myHandle);
}

Semaphore::~Semaphore() {
    sem_close(myHandle);
}

PeriodicThread::PeriodicThread(time_t period) {
    this->period=period;
    finished=false;
}

void PeriodicThread::run() {
    while (!finished) {
        periodicActivation ();
        time_sleep(period);
    }
}

void PeriodicThread::terminate() {
    finished=true;
}

char Console::getc() {
    return ::getc();
}

void Console::putc(char c) {
    ::putc(c);
}
