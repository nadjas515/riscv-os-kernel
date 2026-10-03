//
// Created by os on 6/11/26.
//

#include "../inc/RiscV.hpp"
#include "../inc/Console.hpp"
#include "../inc/Thread.hpp"
#include "../inc/Scheduler.hpp"
#include "../inc/Semaphore.hpp"

uint64 RiscV::kernelStackTop=0;

void RiscV::handleSupervisorTrap(uint64 *regs) {
    uint64 scause = r_scause();

    if (scause == 0x0000000000000008UL || scause == 0x0000000000000009UL) {
        Kernel::Thread::running->setSepc(r_sepc() + 4);
        uint64 code = regs[10];
        switch (code) {
            case 0x1: {
                size_t size = regs[11];
                size = size * MEM_BLOCK_SIZE;
                regs[10] = (uint64) MemoryAllocator::getInstance()->allocateMemory(size);
                break;
            }
            case 0x2: {
                void *ptr = (void *) regs[11];
                regs[10] = (uint64) MemoryAllocator::getInstance()->freeMemory(ptr);
                break;
            }
            case 0x11: {
                Kernel::Thread **handle = (Kernel::Thread **) regs[11];
                Kernel::func start_routine = (Kernel::func) regs[12];
                void *arg = (void *) regs[13];
                void *stackSpace = (void *) regs[14];
                int res = Kernel::Thread::createThread(handle, start_routine, arg, stackSpace);
                if (res == 0) {
                    Kernel::Scheduler::put(*handle);
                }

                regs[10] = res;
                break;
            }
            case 0x12: {
                Kernel::Thread::running->setFinished(true);
                regs[10]=Kernel::Thread::exit();
                Kernel::Thread::running->setSstatus(r_sstatus());
                Kernel::Thread::dispatch();
                w_sstatus(Kernel::Thread::running->getSstatus());
                break;
            }
            case 0x13: {
                Kernel::Thread::running->setSstatus(r_sstatus());
                Kernel::Thread::dispatch();
                w_sstatus(Kernel::Thread::running->getSstatus());
                break;
            }
            case 0x21: {
                Kernel::Semaphore** handle=(Kernel::Semaphore**)regs[11];
                unsigned init=(unsigned)regs[12];
                regs[10]=Kernel::Semaphore::openSemaphore(handle,init);
                break;
            }
            case 0x22: {
                Kernel::Semaphore* sem=(Kernel::Semaphore*)regs[11];
                regs[10]=Kernel::Semaphore::closeSemaphore(sem);
                break;
            }
            case 0x23: {
                Kernel::Semaphore* sem=(Kernel::Semaphore*)regs[11];
                if (!sem || !Kernel::Semaphore::checkIfExists(sem))
                    regs[10]=-1;
                else {
                    Kernel::Thread::running->setSstatus(r_sstatus());
                    regs[10]=sem->wait(1);
                    w_sstatus(Kernel::Thread::running->getSstatus());
                }
                break;
            }
            case 0x24: {
                Kernel::Semaphore* sem=(Kernel::Semaphore*)regs[11];
                if (!sem || !Kernel::Semaphore::checkIfExists(sem))
                    regs[10]=-1;
                else {
                    regs[10]=0;
                    sem->signal(1);
                }
                break;
            }
            case 0x25: {
                Kernel::Semaphore* sem=(Kernel::Semaphore*)regs[11];
                unsigned n=(unsigned)regs[12];
                if (!sem || !Kernel::Semaphore::checkIfExists(sem))
                    regs[10]=-1;
                else {
                    Kernel::Thread::running->setSstatus(r_sstatus());
                    regs[10]=sem->wait(n);
                    w_sstatus(Kernel::Thread::running->getSstatus());
                }
                break;
            }
            case 0x26: {
                Kernel::Semaphore* sem=(Kernel::Semaphore*)regs[11];
                unsigned n=(unsigned)regs[12];
                if (!sem || !Kernel::Semaphore::checkIfExists(sem))
                    regs[10]=-1;
                else {
                    regs[10]=0;

                    sem->signal(n);
                }
                break;
            }
            case 0x31: {
                time_t time=regs[11];
                Kernel::Thread::putToSleep(time);
                regs[10]=0;
                break;
            }
            case 0x41: {
                regs[10]=Kernel::Console::getInstance()->getc();
                break;
            }
            case 0x42: {
                char c=(char)regs[11];
                Kernel::Console::getInstance()->putc(c);
                break;
            }
            default: {
                Kernel::Thread::running->setSstatus(r_sstatus());
                Kernel::Thread::timeSliceCounter = 0;
                Kernel::Thread::dispatch();
                w_sstatus(Kernel::Thread::running->getSstatus());
            }
        }
        w_sepc(Kernel::Thread::running->getSepc());
    } else if (scause == 0x8000000000000001UL) {
        Kernel::Thread::timeSliceCounter++;
        Kernel::Thread::wakeUpThreads();
        if (Kernel::Thread::timeSliceCounter >= Kernel::Thread::running->getTimeSlice()) {
            Kernel::Thread::running->setSepc(r_sepc());
            Kernel::Thread::running->setSstatus(r_sstatus());
            Kernel::Thread::dispatch();
            w_sstatus(Kernel::Thread::running->getSstatus());
            w_sepc(Kernel::Thread::running->getSepc());
        }
        __asm__ volatile ("csrc sip, %0" : : "r" (0x2));
    } else if (scause == 0x8000000000000009UL) {
        int irq = plic_claim();
        if (irq==CONSOLE_IRQ) {
            uint8 status=*(volatile uint8*)CONSOLE_STATUS;
            if (status & CONSOLE_RX_STATUS_BIT) {
                Kernel::Console::getInstance()->handle_interrupt();
            }
            plic_complete(irq);
        }
    } else {
        // unknown trap cause
    }
}


void RiscV::popSppSpie() {
    __asm__ volatile("csrs sstatus, %0" :: "r"(SSTATUS_SPIE));
    __asm__ volatile("csrw sepc,ra");
    __asm__ volatile("sret");
}
