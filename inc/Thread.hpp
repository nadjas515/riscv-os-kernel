//
// Created by os on 6/6/26.
//

#ifndef PROJECT_BASE_V1_1_PCB_H
#define PROJECT_BASE_V1_1_PCB_H
#include "../lib/hw.h"
#include "RiscV.hpp"
#include "List.hpp"
#include "SleepList.hpp"

namespace Kernel {
    typedef void (*func)(void*);
    class Thread {
    public:
        friend class ::RiscV;
        static int createThread(Thread** handle,func function,void* arg,void* stackSpace,uint64 timeSlice=DEFAULT_TIME_SLICE,bool kernelThread=false);
        Thread()=default;
        ~ Thread();

        bool getFinished(){return finished;}
        void setFinished(bool finished){this->finished=finished;}

        uint64 getTimeSlice(){return timeSlice;}

        static void yield();
        static void dispatch();
        static Thread* running;
        static Thread* idle;
        static void initIdle();
        static int exit();
        static void cleanup();
        static void jump();

        static void wakeUpThreads();

        Thread(func function,void* arg,uint64 timeSlice=DEFAULT_TIME_SLICE);

        void setWaiting(int n){waiting=n;}
        int getWaiting() const{return waiting;}

        void setSepc(uint64 sepc){context.sepc=sepc;}
        uint64 getSepc(){return context.sepc;}
        void setSstatus(uint64 sstatus){context.sstatus=sstatus;}
        uint64 getSstatus(){return context.sstatus;}

        uint64 getKernelStackTop(){return (uint64)kernelStack+ DEFAULT_STACK_SIZE;}

        static void putToSleep(time_t time);

        static bool hasActiveThreads(){return activeThreads>0;}
    private:
        static SleepList<Thread> sleepyThreads;
        static List<Thread> zombieThreads;
        static void threadWrapper();


        struct Context {
            uint64 ra;
            uint64 sp;
            uint64 s[12];
            uint64 sepc;
            uint64 sstatus;
        };
        static void contextSwitch(Context* oldContext,Context* newContext);

        func function;
        void* arg;
        bool finished;
        uint64 timeSlice;
        uint64* stack;
        uint64* kernelStack;
        Context context;
        int waiting;
        bool kernelThread;

        static uint64 timeSliceCounter;

        static uint64 activeThreads;
    };
}

#endif //PROJECT_BASE_V1_1_PCB_H
