//
// Created by os on 6/20/26.
//
//
// Sorted (ascending) list of sleeping threads.
// Each element stores its time RELATIVE to the previous element in the list
// (not the absolute remaining time), as described in the project specification.
//
#ifndef PROJECT_BASE_V1_1_SLEEPLIST_H
#define PROJECT_BASE_V1_1_SLEEPLIST_H
#include "MemoryAllocator.hpp"
#include "syscall_c.h"
template <typename T>
class SleepList {
public:
    SleepList():head(0){}
    SleepList(const SleepList<T>&)=delete;
    SleepList<T> &operator=(const SleepList<T> &) = delete;
    void add(T* data, uint64 relativeTime) {
        Elem* elem=(Elem*)MemoryAllocator::getInstance()->allocateMemory(sizeof(Elem));
        elem->data = data;

        Elem* prev = 0;
        Elem* curr = head;
        uint64 remaining = relativeTime;

        while (curr && curr->time <= remaining) {
            remaining -= curr->time;
            prev = curr;
            curr = curr->next;
        }

        elem->time = remaining;
        elem->next = curr;

        if (curr) {
            // The next element now has less remaining time (relative to the NEW element)
            curr->time -= remaining;
        }

        if (prev) {
            prev->next = elem;
        } else {
            head = elem;
        }
    }

    void tick() {
        if (head && head->time > 0) {
            head->time--;
        }
    }

    T* popReady() {
        if (head && head->time == 0) {
            Elem* elem = head;
            T* ret = elem->data;
            head = elem->next;
            MemoryAllocator::getInstance()->freeMemory(elem);
            return ret;
        }
        return 0;
    }

    bool empty() const { return head == 0; }

private:
    struct Elem {
        T* data;
        uint64 time;   // time relative to the PREVIOUS element (or from now, for the head)
        Elem* next;
    };
    Elem* head;
};

#endif //PROJECT_BASE_V1_1_SLEEPLIST_H
