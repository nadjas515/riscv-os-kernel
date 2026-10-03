//
// Created by os on 6/14/26.
//
#include "../inc/Semaphore.hpp"
#include "../inc/MemoryAllocator.hpp"
#include "../inc/Scheduler.hpp"

List<Kernel::Semaphore> Kernel::Semaphore::activeSems;

int Kernel::Semaphore::openSemaphore(Semaphore** handle,unsigned init) {
    if (!handle)
        return -1;
    Semaphore* semaphore=(Semaphore*)MemoryAllocator::getInstance()->allocateMemory(sizeof(Semaphore));
    semaphore->sstatus=0;
    semaphore->val=init;
    semaphore->blocked.init();
    *handle=semaphore;

    activeSems.addLast(semaphore);

    return 0;
}

int Kernel::Semaphore::closeSemaphore(Semaphore* id) {
    bool res=activeSems.remove(id);
    if (!res)
        return -1;
    while (Thread* t=id->blocked.removeFirst())
        Scheduler::put(t);
    MemoryAllocator::getInstance()->freeMemory(id);
    return 0;
}

bool Kernel::Semaphore::checkIfExists(Semaphore* id) {
    return activeSems.check(id);
}

Kernel::Semaphore::Semaphore(int init):sstatus(0),val(init) {}

Kernel::Semaphore::~Semaphore() {}

void Kernel::Semaphore::block() {
    blocked.addLast(Thread::running);

    unlock();
    Thread::jump();
}

void Kernel::Semaphore::deblock() {
    if (blocked.peekFirst())
        Scheduler::put(blocked.removeFirst());
}

void Kernel::Semaphore::lock() {
    sstatus=RiscV::r_sstatus();
    RiscV::mc_sstatus(RiscV::SSTATUS_SIE);
}

void Kernel::Semaphore::unlock() {
    RiscV::ms_sstatus(sstatus & RiscV::SSTATUS_SIE ? RiscV::SSTATUS_SIE : 0);
}


int Kernel::Semaphore::wait(unsigned n) {
    lock();
    val -= n;
    if (val<0) {
        Thread::running->setWaiting(-val);
        val = 0;
        block();
        if (Thread::running->getWaiting())
            return -2;
        return 0;
    }
    unlock();
    return 0;
}

void Kernel::Semaphore::signal(unsigned n) {
    lock();
    val+=n;
    while (true) {
        Thread* thread=blocked.peekFirst();
        if (!thread)
            break;
        int waiting=thread->getWaiting();
        if (val>=waiting) {
            val-=waiting;
            thread->setWaiting(0);
            deblock();
        }
        else {
            thread->setWaiting(waiting - val);
            val=0;
            break;
        }
    }
    unlock();
}