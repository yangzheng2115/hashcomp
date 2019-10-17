//
// Created by Michael on 2019-10-12.
//


#include <atomic>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <thread>
#include "gtest/gtest.h"
#include "MaskRWPtr.h"
#include "tracer.h"

using namespace std;

uint64_t TotalPayloads = (1LLU << 24);
uint64_t ThreadsNumber = 4;
uint64_t LoadPerThread = TotalPayloads / ThreadsNumber;
int usingSub = true;

atomic<uint64_t> actualCount(0);

AtomicMaskUint64 amb;

MaskUint64 mu;

void MaskUint64Add() {
    uint64_t tick = 0;
    for (uint64_t i = 0; i < LoadPerThread; i++) {
        atomic_add(&mu.dword, 1);
        if (usingSub) atomic_add(&mu.dword, -1);
        tick++;
    }
    actualCount.fetch_add(tick);
}

void MaskByteAdd(int &idx) {
    uint64_t tick = 0;
    for (uint64_t i = 0; i < LoadPerThread; i++) {
        atomic_mask_add(&mu, idx % 8, 1);
        if (usingSub) atomic_mask_sub(&mu, idx % 8, 1);
        tick++;
    }
    actualCount.fetch_add(tick);
}

void DWordAdd() {
    uint64_t tick = 0;
    for (uint64_t i = 0; i < LoadPerThread; i++) {
        amb.dword.fetch_add(1);
        if (usingSub) amb.dword.fetch_sub(1);
        tick++;
    }
    actualCount.fetch_add(tick);
}

// use macro instead
void ByteAdd(int &idx) {
    uint64_t tick = 0;
    for (uint64_t i = 0; i < LoadPerThread; i++) {
        switch (idx % 8) {
            case 0:
                amb.bytes.byte0.fetch_add(1);
                if (usingSub) amb.bytes.byte0.fetch_sub(1);
                break;
            case 1:
                amb.bytes.byte1.fetch_add(1);
                if (usingSub) amb.bytes.byte1.fetch_sub(1);
                break;
            case 2:
                amb.bytes.byte2.fetch_add(1);
                if (usingSub) amb.bytes.byte2.fetch_sub(1);
                break;
            case 3:
                amb.bytes.byte3.fetch_add(1);
                if (usingSub) amb.bytes.byte3.fetch_sub(1);
                break;
            case 4:
                amb.bytes.byte4.fetch_add(1);
                if (usingSub) amb.bytes.byte4.fetch_sub(1);
                break;
            case 5:
                amb.bytes.byte5.fetch_add(1);
                if (usingSub) amb.bytes.byte5.fetch_sub(1);
                break;
            case 6:
                amb.bytes.byte6.fetch_add(1);
                if (usingSub) amb.bytes.byte6.fetch_sub(1);
                break;
            case 7:
                amb.bytes.byte7.fetch_add(1);
                if (usingSub) amb.bytes.byte7.fetch_sub(1);
                break;
            default:
                return;
        }
        tick++;
    }
    actualCount.fetch_add(tick);
}

int main(int argc, char **argv) {
    if (argc > 3) {
        ThreadsNumber = std::atol(argv[1]);
        TotalPayloads = std::atol(argv[2]);
        usingSub = std::atoi(argv[3]);
        LoadPerThread = TotalPayloads / ThreadsNumber;
        cout << "Thread: " << argv[1] << " total: " << argv[2] << " sub: " << argv[3] << endl;
    }
    cout << "Thread: " << ThreadsNumber << " total: " << TotalPayloads << " sub: " << usingSub << endl;
    amb.dword = 0;
    Tracer tracer;
    tracer.startTime();

    thread workers[ThreadsNumber];
    for (int i = 0; i < ThreadsNumber; i++) {
        workers[i] = thread(DWordAdd);
    }
    for (int i = 0; i < ThreadsNumber; i++) {
        workers[i].join();
    }
    cout << "DWord: \t" << tracer.getRunTime() << "<->" << actualCount.load() << ":" << amb.dword << endl;

    actualCount.store(0);
    amb.dword.store(0);
    tracer.startTime();

    thread byters[ThreadsNumber];
    for (int i = 0; i < ThreadsNumber; i++) {
        byters[i] = thread(ByteAdd, ref(i));
    }
    for (int i = 0; i < ThreadsNumber; i++) {
        byters[i].join();
    }
    cout << "Bytes: \t" << tracer.getRunTime() << "<->" << actualCount.load() << ":" << amb.dword << endl;

    actualCount.store(0);
    mu.dword = 0;
    tracer.startTime();

    thread uint64er[ThreadsNumber];
    for (int i = 0; i < ThreadsNumber; i++) {
        uint64er[i] = thread(MaskUint64Add);
    }
    for (int i = 0; i < ThreadsNumber; i++) {
        uint64er[i].join();
    }
    cout << "Uint64: " << tracer.getRunTime() << "<->" << actualCount.load() << ":" << mu.dword << endl;

    actualCount.store(0);
    mu.dword = 0;
    tracer.startTime();

    thread cbyters[ThreadsNumber];
    for (int i = 0; i < ThreadsNumber; i++) {
        cbyters[i] = thread(MaskByteAdd, ref(i));
    }
    for (int i = 0; i < ThreadsNumber; i++) {
        cbyters[i].join();
    }
    cout << "CByte: \t" << tracer.getRunTime() << "<->" << actualCount.load() << ":" << mu.dword << endl;

    return 0;
}