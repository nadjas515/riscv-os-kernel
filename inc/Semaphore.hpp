//
// Created by os on 6/14/26.
//

#ifndef PROJECT_BASE_V1_1_SEMAPHORE_H
#define PROJECT_BASE_V1_1_SEMAPHORE_H
#include "List.hpp"
#include "Thread.hpp"

namespace Kernel {
    class Semaphore {
    public:
        static int openSemaphore(Semaphore** handle,unsigned init=1);
        static int closeSemaphore(Semaphore* id);
        static bool checkIfExists(Semaphore* id);
        Semaphore(int init=1);
        ~Semaphore();
        int wait(unsigned n=1);
        void signal(unsigned n=1);
    protected:
        void block();
        void deblock();
        void lock();
        void unlock();
    private:
        static List<Semaphore> activeSems;
        uint64 sstatus;
        int val;
        List<Kernel::Thread> blocked;
    };
}


#endif //PROJECT_BASE_V1_1_SEMAPHORE_H
