//
// Created by os on 6/19/26.
//

#ifndef PROJECT_BASE_V1_1_CONSOLE_H
#define PROJECT_BASE_V1_1_CONSOLE_H
#include "Thread.hpp"
#include "Semaphore.hpp"

namespace Kernel {
    class Console {
    public:
        static Console* getInstance();
        char getc();
        void putc(char c);
        void handle_interrupt();
        bool txEmpty();
    private:
        static void txFunction(void* arg);
        static void rxFunction();
        Kernel::Thread* txThread;
        struct Buffer {
            char buffer[256];
            int head, tail;
            Semaphore *write,*read;
        };
        Buffer TX,RX;
        Console();
    };
}

#endif //PROJECT_BASE_V1_1_CONSOLE_H
