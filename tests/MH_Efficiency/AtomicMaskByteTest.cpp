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

uint64_t TotalPayloads = (1LLU << 20);
uint64_t ThreadsNumber = 4;
uint64_t LoadPerThread = TotalPayloads / ThreadsNumber;
int usingSub = true;

AtomicMaskByte amb;

void DWordAdd() {
    for (uint64_t i = 0; i < LoadPerThread; i++) {
        amb.dword.fetch_add(1);
        if (usingSub) amb.dword.fetch_sub(1);
    }
}

// use macro instead
void ByteAdd(int &idx) {
    for (uint64_t i = 0; i < LoadPerThread; i++) {
        switch (idx % 8) {
            case 0:
                amb.byte0.fetch_add(1);
                if (usingSub) amb.byte0.fetch_sub(1);
                break;
            case 1:
                amb.byte1.fetch_add(1);
                if (usingSub) amb.byte1.fetch_sub(1);
                break;
            case 2:
                amb.byte2.fetch_add(1);
                if (usingSub) amb.byte2.fetch_sub(1);
                break;
            case 3:
                amb.byte3.fetch_add(1);
                if (usingSub) amb.byte3.fetch_sub(1);
                break;
            case 4:
                amb.byte4.fetch_add(1);
                if (usingSub) amb.byte4.fetch_sub(1);
                break;
            case 5:
                amb.byte5.fetch_add(1);
                if (usingSub) amb.byte5.fetch_sub(1);
                break;
            case 6:
                amb.byte6.fetch_add(1);
                if (usingSub) amb.byte6.fetch_sub(1);
                break;
            case 7:
                amb.byte7.fetch_add(1);
                if (usingSub) amb.byte7.fetch_sub(1);
                break;
            default:
                return;
        }
    }
}

int main(int argc, char **argv) {
    if (argc > 3) {
        ThreadsNumber = std::atol(argv[1]);
        TotalPayloads = std::atol(argv[2]);
        usingSub = std::atoi(argv[3]);
    }
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
    cout << "DWord: " << tracer.getRunTime() << endl;

    tracer.startTime();

    thread byters[ThreadsNumber];
    for (int i = 0; i < ThreadsNumber; i++) {
        byters[i] = thread(ByteAdd, ref(i));
    }
    for (int i = 0; i < ThreadsNumber; i++) {
        byters[i].join();
    }
    cout << "Bytes: " << tracer.getRunTime() << endl;
    return 0;
}