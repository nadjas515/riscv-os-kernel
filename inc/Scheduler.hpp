//
// Created by os on 6/10/26.
//

#ifndef PROJECT_BASE_V1_1_SCHEDULER_H
#define PROJECT_BASE_V1_1_SCHEDULER_H
#include "List.hpp"

namespace Kernel {
    class Thread;

    class Scheduler {
    private:
        static List<Thread> readyThreadQueue;

    public:
        static Thread *get();

        static void put(Thread *thread);

    };
}

#endif //PROJECT_BASE_V1_1_SCHEDULER_H
