//
// Created by os on 6/5/26.
//

#ifndef PROJECT_BASE_V1_1_MEMORYALLOCATOR_H
#define PROJECT_BASE_V1_1_MEMORYALLOCATOR_H
#include "syscall_c.h"
typedef struct FreeMem {
    struct FreeMem* next; // Next in the list
    struct FreeMem* prev; // Previous in the list
    size_t size; // Size of the free fragment
} Memory;


class MemoryAllocator {
public:
    static MemoryAllocator* getInstance();
    void* allocateMemory (size_t size);
    int freeMemory (void* memory);
private:
    MemoryAllocator();
    void join(Memory *prev);
    Memory* first;
};
#endif //PROJECT_BASE_V1_1_MEMORYALLOCATOR_H
