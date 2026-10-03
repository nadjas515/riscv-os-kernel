//
// Created by os on 6/10/26.
//
#include "../inc/Scheduler.hpp"
#include "../inc/Thread.hpp"
#include "../inc/List.hpp"

List<Kernel::Thread> Kernel::Scheduler::readyThreadQueue;

Kernel::Thread *Kernel::Scheduler::get(){
    Thread* t= readyThreadQueue.removeFirst();

    if (!t)
        return Thread::idle;
    return t;
}

void Kernel::Scheduler::put(Kernel::Thread *thread){
    readyThreadQueue.addLast(thread);
}
