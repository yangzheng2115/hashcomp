//
// Created by iclab on 9/25/19.
//
#include <chrono>
#include <iostream>
#include <sstream>
#include <memory>
#include <thread>
#include <vector>
#include "tracer.h"

using namespace std;

int pdegree = 4;

constexpr long iteration = (1LLU << 30);

#define STORE_TEST 1

#define AL_ALONE 0

atomic<long> *alp;

atomic<long> al;

int pstep = 64;

stringstream *output;

void loader(int tid) {
    long v = -1;
    for (long i = 0; i < iteration / pdegree; i++) {
#if STORE_TEST
#if AL_ALONE
        al.store(i);
#else
        alp[tid * pstep].store(i);
#endif
#else
#if AL_ALONE
        v = al.load();
#else
        v = alp[tid * pstep].load();
#endif
#endif
    }
}

void multiThreadLoadTests() {
    delete[] output;
    output = new stringstream[pdegree];
    alp = new atomic<long>[pdegree * pstep];
    for (int i = 0; i < pdegree; i += pstep) {
        alp->store(0);
    }
    std::vector<thread> workers;
    Tracer tracer;
    cout << "Intend: " << iteration << " " << pdegree << endl;
    tracer.startTime();
    for (int t = 0; t < pdegree; t++) {
        workers.push_back(std::thread(loader, t));
    }
    long ec = 0;
    for (int t = 0; t < pdegree; t++) {
        workers[t].join();
        cout << "\t" << output[t].str();
    }

    cout << "Multi-thread: " << pdegree << "<->" << ec << "<>" << ":" << tracer.getRunTime() << endl;
    delete[] alp;
}

int main(int argc, char **argv) {
    output = new stringstream[pdegree];
    multiThreadLoadTests();
    delete[] output;
}