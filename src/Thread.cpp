//
// Created by os on 6/6/26.
//
#include "../inc/Thread.hpp"
#include "../inc/MemoryAllocator.hpp"
#include "../inc/RiscV.hpp"
#include "../inc/Scheduler.hpp"

Kernel::Thread* Kernel::Thread::idle=0;
Kernel::Thread* Kernel::Thread::running=0;
uint64 Kernel::Thread::timeSliceCounter=0;
uint64 Kernel::Thread::activeThreads=0;
List<Kernel::Thread> Kernel::Thread::zombieThreads;
SleepList<Kernel::Thread> Kernel::Thread::sleepyThreads;

void loop(void* arg) {
    while (true) {
        asm volatile ("wfi");
    }
}

void Kernel::Thread::initIdle() {
    void* stackSpace=MemoryAllocator::getInstance()->allocateMemory(DEFAULT_STACK_SIZE);
    stackSpace = (void*)((char*)stackSpace + DEFAULT_STACK_SIZE);
    createThread(&idle, loop, nullptr, stackSpace,0,true);
}

int Kernel::Thread::exit() {
    if (running==idle)
        return -1;
    zombieThreads.addFirst(running);
    if (!running->kernelThread)
        activeThreads--;
    return 0;
}

void Kernel::Thread::cleanup() {
    while(Thread *t=zombieThreads.removeFirst()) {
        t->~Thread();
        MemoryAllocator::getInstance()->freeMemory(t);
    }
}

Kernel::Thread::Thread(func function,void* arg,uint64 timeSlice)
    : function(function),arg(arg),finished(false),timeSlice(timeSlice),waiting(0) {
    kernelStack=(uint64*)MemoryAllocator::getInstance()->allocateMemory(DEFAULT_STACK_SIZE);
    if (function) {
        stack=(uint64*)MemoryAllocator::getInstance()->allocateMemory(DEFAULT_STACK_SIZE);
        context.ra=(uint64)threadWrapper;
        context.sp = (uint64)stack + DEFAULT_STACK_SIZE;
        Scheduler::put(this);
    }
    else {
        stack=0;
        context.ra=0;
        context.sp = 0;
    }
    context.sepc=0;
    context.sstatus=RiscV::SSTATUS_SPP;
    kernelThread=true;
    for (int i = 0; i < 12; i++)
        context.s[i] = 0;
}

int Kernel::Thread::createThread(Kernel::Thread** handle,func function, void *arg,void* stackSpace,uint64 timeSlice,bool kernelThread) {
    if (!handle)
        return -1;

    Thread* thread=(Thread*)MemoryAllocator::getInstance()->allocateMemory(sizeof(Thread));

    if (!thread)
        return -2;

    thread->function = function;
    thread->arg = arg;
    thread->finished = false;
    thread->timeSlice=timeSlice;
    thread->kernelStack=(uint64*)MemoryAllocator::getInstance()->allocateMemory(DEFAULT_STACK_SIZE);

    if (!thread->kernelStack) {
        MemoryAllocator::getInstance()->freeMemory(thread);
        return -3;
    }

    if (function) {
        thread->stack=(uint64*)((uint64)stackSpace-DEFAULT_STACK_SIZE);
        thread->context.ra=(uint64)threadWrapper;
        thread->context.sp = (uint64)stackSpace;
    }
    else {
        thread->stack=0;
        thread->context.ra=0;
        thread->context.sp = 0;
    }

    thread->context.sepc=0;
    if (kernelThread)
        thread->context.sstatus=RiscV::SSTATUS_SPP;
    else
        thread->context.sstatus=0;
    thread->waiting=0;
    thread->kernelThread=kernelThread;
    if (!kernelThread)
        activeThreads++;
    for (int i = 0; i < 12; i++)
        thread->context.s[i] = 0;
    *handle=thread;

    return 0;
}

Kernel::Thread::~Thread() {
    if(stack) {
        MemoryAllocator::getInstance()->freeMemory(stack);
    }
    if (kernelStack) {
        MemoryAllocator::getInstance()->freeMemory(kernelStack);
    }
}

void Kernel::Thread::yield() {
    __asm__ volatile("li a0, 0x13");
    __asm__ volatile("ecall");
}

void Kernel::Thread::dispatch() {
    cleanup();
    Thread* old=running;

    if (!old->finished && old!=idle) {
        Scheduler::put(old);
    }
    running=Scheduler::get();
    timeSliceCounter = 0;
    RiscV::kernelStackTop=running->getKernelStackTop();
    contextSwitch(&old->context,&running->context);
}

void Kernel::Thread::threadWrapper() {
    __asm__ volatile("csrw sscratch, %0" :: "r"(running->getKernelStackTop()));
    RiscV::kernelStackTop = running->getKernelStackTop();
    RiscV::w_sstatus(running->getSstatus());
    RiscV::popSppSpie();
    running->function(running->arg);
    running->finished=true;
    exit();
    yield();
}

void Kernel::Thread::jump() {
    Thread* old=running;
    running=Scheduler::get();
    timeSliceCounter = 0;
    RiscV::kernelStackTop=running->getKernelStackTop();
    contextSwitch(&old->context,&running->context);
}


void Kernel::Thread::putToSleep(time_t time) {
    if (time == 0)
        return;
    sleepyThreads.add(running,time);
    jump();
}

void Kernel::Thread::wakeUpThreads() {
    sleepyThreads.tick();
    while (Thread *t = sleepyThreads.popReady()) {
        Scheduler::put(t);
    }
}