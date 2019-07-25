//
// Created by Michael on 2019-07-26.
//

#include <functional>
#include <iostream>
#include <stack>
#include <thread>
#include <unordered_set>
#include "tracer.h"
#include "ycsbHelper.h"
#include "hasher.h"

char *file_path = "./load.dat";

char **sinput;

uint64_t total = 100;

int pdegree = 4;

int simple = 1;

long *runtime;

void mhasherTests(bool init = true, int tid = 0) {
    Tracer tracer;
    tracer.startTime();
    unordered_set<uint64_t> umap;
    mhasher<const char *> hasher;
    if (init) {
        cout << "\tKey gen: " << tracer.getRunTime() << " with " << total << endl;
    }
    for (size_t i = 0; i < total; i++) {
        char *key = sinput[i];
        uint64_t gk = hasher.hash(key);
        if (gk == -1) {
            umap.insert(gk);
        }
    }
    runtime[tid] = tracer.getRunTime();
    //cout << "\tSet gen: " << tracer.getRunTime() << " with " << umap.size() << endl;
    if (init) {
        for (size_t i = 0; i < total; i++) {
            char *key = sinput[i];
            uint64_t gk = hasher.hash(key);
            umap.insert(gk);
        }
        cout << "\tSet vrf: " << tracer.getRunTime() << " with " << umap.size() << endl;
#define HISTGRAM_BINS 1024
        size_t histgram[HISTGRAM_BINS];
        cout << "********** " << sizeof(histgram) << endl;
        std::memset(histgram, 0, sizeof(histgram));
        for (size_t i = 0; i < total; i++) {
            char *key = sinput[i];
            size_t idx = hasher.hash(key) / (std::numeric_limits<size_t>::max() / HISTGRAM_BINS);
            histgram[idx]++;
        }
        for (int i = 0; i < HISTGRAM_BINS; i++) {
            cout << histgram[i] << "\t";
            if ((i + 1) % 16 == 0) {
                cout << endl;
            }
        }
        cout << "\tHst gen: " << tracer.getRunTime() << " with " << umap.size() << endl;
    }
}

void multiHasherTests() {
    Tracer tracer;
    tracer.startTime();
    std::vector<std::thread> workers;
    for (int t = 0; t < pdegree; t++) {
        workers.push_back(std::thread(mhasherTests, false, t));
    }
    for (int t = 0; t < pdegree; t++) {
        workers[t].join();
        cout << "\t\tThread " << runtime[t] << " with " << total << endl;
    }
    cout << "Multi-thread gen: " << tracer.getRunTime() << " with " << (total * pdegree) << endl;
}

int main(int argc, char **argv) {
    if (argc > 3) {
        total = std::atol(argv[1]);
        pdegree = std::atoi(argv[2]);
        simple = std::atoi(argv[3]);
    }
    if (argc > 4) {
        file_path = argv[4];
    }
    cout << total << " " << pdegree << " " << simple << " " << argv[4] << endl;
    runtime = new long[pdegree];
    Tracer tracer;
    tracer.startTime();
    sinput = new char *[total];
    loadYCSB(file_path, total, sinput);
    cout << "Load time: " << tracer.getRunTime() << endl;

    if (simple) {
        mhasherTests();
        multiHasherTests();
    } else {
        cout << "Simple hash function tests supported." << endl;
    }
    delete[] runtime;
}