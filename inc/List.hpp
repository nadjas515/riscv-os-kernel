//
// Created by os on 6/11/26.
//

#ifndef PROJECT_BASE_V1_1_LIST_H
#define PROJECT_BASE_V1_1_LIST_H
#include "MemoryAllocator.hpp"

template <typename T>
class List {
public:
    List():head(0),tail(0){}
    List(const List<T>&)=delete;
    List<T> &operator=(const List<T> &) = delete;

    void init() {
        head = 0;
        tail = 0;
    }

    void addFirst(T *data)
    {
        Elem* elem=(Elem*)MemoryAllocator::getInstance()->allocateMemory(sizeof(Elem));
        elem->data = data;
        elem->next = head;
        head = elem;
        if (!tail) { tail = head; }
    }

    void addLast(T *data)
    {
        Elem* elem=(Elem*)MemoryAllocator::getInstance()->allocateMemory(sizeof(Elem));

        elem->data = data;
        elem->next = 0;
        if (tail)
        {
            tail->next = elem;
            tail = elem;
        } else
        {
            head = tail = elem;
        }
    }

    T *removeFirst()
    {
        if (!head) { return 0; }

        Elem *elem = head;
        head = head->next;
        if (!head) { tail = 0; }

        T *ret = elem->data;
        MemoryAllocator::getInstance()->freeMemory(elem);
        return ret;
    }

    T *peekFirst()
    {
        if (!head) { return 0; }
        return head->data;
    }
    T *removeLast()
    {
        if (!head) { return 0; }

        Elem *prev = 0;
        for (Elem *curr = head; curr && curr != tail; curr = curr->next)
        {
            prev = curr;
        }

        Elem *elem = tail;
        if (prev) { prev->next = 0; }
        else { head = 0; }
        tail = prev;

        T *ret = elem->data;
        MemoryAllocator::getInstance()->freeMemory(elem);
        return ret;
    }

    T *peekLast()
    {
        if (!tail) { return 0; }
        return tail->data;
    }

    bool check(T* data)
    {
        Elem *curr;
        for (curr = head; curr && curr->data!=data; curr = curr->next);
        return curr!=0;
    }

    bool remove(T* data)
    {
        Elem *curr,*prev=0;
        for (curr = head; curr && curr->data!=data; prev=curr,curr = curr->next);
        if (!curr) { return false; }
        if (!prev) {
            head=curr->next;
            if (!head)
                tail=0;
        }
        else {
            prev->next = curr->next;
            if (!prev->next)
                tail=prev;
        }
        curr->next=0;
        MemoryAllocator::getInstance()->freeMemory(curr);
        return true;
    }

    T* getIterator() {
        iterator = head;
        if (!iterator)
            return 0;
        return iterator->data;
    }

    T* getNext() {
        if (!iterator || !iterator->next)
            return 0;
        iterator=iterator->next;
        return iterator->data;
    }

private:
    struct Elem {
        T* data;
        Elem* next;
        Elem(T *data,Elem* next): data(data), next(next) {}
    };
    Elem* head,*tail;
    Elem* iterator;
};



#endif //PROJECT_BASE_V1_1_LIST_H
