//
// Created by os on 6/19/26.
//
#include "../inc/Console.hpp"
#include "../inc/MemoryAllocator.hpp"
#include "../inc/Scheduler.hpp"
#include "../lib/console.h"

Kernel::Console* Kernel::Console::getInstance() {
    static Console instance;
    return &instance;
}

Kernel::Console::Console() {
    TX.head=TX.tail=0;
    RX.head=RX.tail=0;
    void* stackSpace=MemoryAllocator::getInstance()->allocateMemory(DEFAULT_STACK_SIZE);
    stackSpace = (void*)((char*)stackSpace + DEFAULT_STACK_SIZE);
    Thread::createThread(&txThread,txFunction,0,stackSpace,DEFAULT_TIME_SLICE,true);
    Scheduler::put(txThread);
    Semaphore::openSemaphore(&RX.read,0);
    Semaphore::openSemaphore(&RX.write,256);
    Semaphore::openSemaphore(&TX.write,256);
    Semaphore::openSemaphore(&TX.read,0);
}

char Kernel::Console::getc() {
    RX.read->wait();
    char c=RX.buffer[RX.tail];
    RX.tail=(RX.tail+1)%256;
    RX.write->signal();
    return c;
}

void Kernel::Console::putc(char c) {
    TX.write->wait();
    TX.buffer[TX.head]=c;
    TX.head=(TX.head+1)%256;
    TX.read->signal();
}

void Kernel::Console::txFunction(void* arg) {
    Console* console = getInstance();
    while (true) {
        console->TX.read->wait();
        char c=console->TX.buffer[console->TX.tail];
        console->TX.tail=(console->TX.tail+1)%256;
        console->TX.write->signal();
        while (!(*(volatile uint8*)CONSOLE_STATUS & CONSOLE_TX_STATUS_BIT))
            Thread::dispatch();
        *(volatile uint8*)CONSOLE_TX_DATA = c;

    }
}

void Kernel::Console::handle_interrupt() {
    rxFunction();
}

void Kernel::Console::rxFunction() {
    Console* console = getInstance();
    console->RX.write->wait();
    char c=*(volatile uint8*)CONSOLE_RX_DATA;
    console->RX.buffer[console->RX.head]=c;
    console->RX.head=(console->RX.head+1)%256;
    console->RX.read->signal();
}

bool Kernel::Console::txEmpty() {
    return TX.head==TX.tail;
}
