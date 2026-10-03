//
// Created by os on 6/7/26.
//
#include "../lib/hw.h"
#include "../inc/Thread.hpp"
#include "../lib/console.h"

extern "C" void* mem_alloc(size_t size) {
    size=(size+MEM_BLOCK_SIZE-1)/MEM_BLOCK_SIZE;
    void* ptr;
    __asm__ volatile("mv a1,%0"::"r"(size));
    __asm__ volatile("li a0,0x01");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ptr));
    return ptr;
}

extern "C" int mem_free(void* ptr) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(ptr));
    __asm__ volatile("li a0,0x02");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

extern "C" int thread_create(thread_t* handle, void(*start_routine)(void*), void* arg) {
    int ret;
    void* stackSpace = mem_alloc(DEFAULT_STACK_SIZE);  // allocate the stack BEFORE the ecall
    if (!stackSpace)
        return -4;
    stackSpace = (void*)((char*)stackSpace + DEFAULT_STACK_SIZE);
    __asm__ volatile("mv a4,%0"::"r"(stackSpace));
    __asm__ volatile("mv a3,%0"::"r"(arg));
    __asm__ volatile("mv a2,%0"::"r"(start_routine));
    __asm__ volatile("mv a1,%0"::"r"(handle));
    __asm__ volatile("li a0,0x11");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

extern "C" int thread_exit () {
    int ret;
    __asm__ volatile("li a0,0x12");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

extern "C" void thread_dispatch () {
    __asm__ volatile("li a0,0x13");
    __asm__ volatile("ecall");
}

extern "C" int sem_open(sem_t* handle, unsigned init) {
    int ret;
    __asm__ volatile("mv a2,%0"::"r"(init));
    __asm__ volatile("mv a1,%0"::"r"(handle));
    __asm__ volatile("li a0,0x21");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}


extern "C" int sem_close (sem_t handle) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(handle));
    __asm__ volatile("li a0,0x22");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

extern "C" int sem_wait (sem_t id) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(id));
    __asm__ volatile("li a0,0x23");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

extern "C" int sem_signal (sem_t id) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(id));
    __asm__ volatile("li a0,0x24");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

extern "C" int sem_wait_n(sem_t id, unsigned n) {
    int ret;
    __asm__ volatile("mv a2,%0"::"r"(n));
    __asm__ volatile("mv a1,%0"::"r"(id));
    __asm__ volatile("li a0,0x25");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

extern "C" int sem_signal_n(sem_t id, unsigned n) {
    int ret;
    __asm__ volatile("mv a2,%0"::"r"(n));
    __asm__ volatile("mv a1,%0"::"r"(id));
    __asm__ volatile("li a0,0x26");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

extern "C" int time_sleep(time_t time) {
    int ret;
    __asm__ volatile("mv a1,%0"::"r"(time));
    __asm__ volatile("li a0,0x31");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

extern "C" char getc() {
    char ret;
    __asm__ volatile("li a0,0x41");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0,a0":"=r"(ret));
    return ret;
}

extern "C" void putc(char c) {
    __asm__ volatile("mv a1,%0"::"r"(c));
    __asm__ volatile("li a0,0x42");
    __asm__ volatile("ecall");
}