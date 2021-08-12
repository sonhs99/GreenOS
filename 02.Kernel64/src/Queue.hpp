#pragma once

#include "Types.hpp"

#pragma pack(push, 1)

template<class T, int buffer_size>
class Queue {
    int iPutIndex;
    int iGetIndex;
    bool bLastOperationPut;

    T Buffer[buffer_size];
public:
    using type = T;

    Queue() : iPutIndex(0), iGetIndex(0), bLastOperationPut(false) {};
    bool isFull() { return iGetIndex == iPutIndex && bLastOperationPut; }
    bool isEmpty() { return iGetIndex == iPutIndex && !bLastOperationPut; }

    bool Enqueue(const T& pvData) {
        if(isFull()) return false;
        Buffer[iPutIndex++] = pvData;
        iPutIndex %= buffer_size;
        bLastOperationPut = true;
        return true;
    }

    bool Dequeue(T& pvData) {
        if(isEmpty()) return false;
        pvData = Buffer[iGetIndex++];
        iGetIndex %= buffer_size;
        bLastOperationPut = false;
        return true;
    }
};

#pragma pack(pop)