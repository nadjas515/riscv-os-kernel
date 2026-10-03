//
// Created by os on 6/5/26.
//
#include "../lib/console.h"
#include "../inc/MemoryAllocator.hpp"

MemoryAllocator* MemoryAllocator::getInstance() {
    static MemoryAllocator instance;
    return &instance;
}

MemoryAllocator::MemoryAllocator() {
    first=(Memory*)HEAP_START_ADDR;
    first->prev=0;
    first->next=0;
    first->size = (size_t)HEAP_END_ADDR-(size_t)HEAP_START_ADDR;
}

void* MemoryAllocator::allocateMemory (size_t size) {
    size_t sz = ((size + sizeof(Memory) + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE) * MEM_BLOCK_SIZE;
    Memory* memBlock = first;
    for(;memBlock && memBlock->size < sz; memBlock = memBlock->next);
    if (memBlock) {
        if (memBlock->size - sz < MEM_BLOCK_SIZE) {
            if (memBlock==first)
                first=memBlock->next;
            if (memBlock->next)
                memBlock->next->prev = memBlock->prev;
            if (memBlock->prev)
                memBlock->prev->next = memBlock->next;
            memBlock->next= memBlock->prev = nullptr;
        }
        else {
            Memory* newMemBlock = (Memory*)((char*)memBlock+sz);
            newMemBlock->prev = memBlock->prev;
            newMemBlock->next = memBlock->next;
            if (memBlock->next)
                memBlock->next->prev = newMemBlock;
            if (memBlock->prev)
                memBlock->prev->next = newMemBlock;
            newMemBlock->size = memBlock->size - sz;
            memBlock->size = sz;
            if (memBlock==first)
                first=newMemBlock;
        }
        return (void*)((char*)memBlock+sizeof(Memory));
    }
    return 0;
}

int MemoryAllocator::freeMemory (void* memory) {
    if (!memory) return -1;
    if ((size_t)memory<sizeof(Memory) || (size_t)memory-sizeof(Memory)<(size_t)HEAP_START_ADDR) return -2;
    if ((size_t)memory>(size_t)HEAP_END_ADDR) return -3;
    memory = (void*)((size_t)memory-sizeof(Memory));
    Memory* prev=nullptr, * next=nullptr;
    if (memory < first) {
        next=first;
    }
    else {
        for (Memory* memBlock = first; memBlock; memBlock = memBlock->next) {
            next=memBlock;
            if (memBlock>memory) break;
        }
        if (next<memory) {
            prev=next;
            next=nullptr;
        }
        else
            prev=next->prev;
    }
    Memory* newMemBlock = (Memory*)memory;
    newMemBlock->prev = prev;
    newMemBlock->next = next;
    if (!prev)
        first=newMemBlock;
    else
        prev->next=newMemBlock;
    if (next)
        next->prev=newMemBlock;
    join(newMemBlock);
    join(prev);
    return 0;
}

void MemoryAllocator::join(Memory *prev) {
    if (prev==nullptr) return;
    if (prev->next==nullptr) return;
    Memory* next=prev->next;
    if ((char*)prev+prev->size==(char*)next) {
        prev->size+=next->size;
        prev->next=next->next;
        if (next->next)
            next->next->prev=prev;
    }
}