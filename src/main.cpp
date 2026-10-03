//
// Created by os on 4/5/26.
//
#include "../inc/syscall_c.h"
#include "../inc/Thread.hpp"
#include "../inc/Console.hpp"
#include "../inc/RiscV.hpp"

extern void userMain();


int main() {
    RiscV::w_stvec((uint64)RiscV::supervisorTrap);
    Kernel::Thread* mainThread;
    Kernel::Thread::createThread(&mainThread,0,0,0,DEFAULT_TIME_SLICE,1);
    Kernel::Thread::running = mainThread;
    RiscV::kernelStackTop = Kernel::Thread::running->getKernelStackTop();
    Kernel::Thread::initIdle();
    __asm__ volatile("csrw sscratch, %0" :: "r"(Kernel::Thread::running->getKernelStackTop()));
    RiscV::ms_sstatus(RiscV::SSTATUS_SPIE);
    userMain();
    while (Kernel::Thread::hasActiveThreads() || !Kernel::Console::getInstance()->txEmpty()) {
        thread_dispatch();
    }
    *(volatile uint32*)0x100000=0x5555;
    return 0;
}
