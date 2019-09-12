//
// Created by iclab on 9/9/19.
//
#include <iostream>
#include <sstream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <unordered_set>
#include "tracer.h"
#include "lock_free_hash_table.h"
#include "concurrent_hash_table.h"
#include "basic_hash_table.h"
#include "atomic_shared_ptr.h"
#include "universal_hash_table.h"

#define DEFAULT_STORE_BASE (1LLU << 28)

#define DEFAULT_STR_LENGTH 256
//#define DEFAULT_KEY_LENGTH 8

#define TEST_LOOKUP        0

#define COUNT_HASH         1

//#define DEFAULT_STORE_BASE 100000000

#if COUNT_HASH == 1
#define UNSAFE
#ifdef UNSAFE
neatlib::ConcurrentHashTable<uint64_t, uint64_t,
        std::hash<uint64_t>, 4, 16,
        neatlib::unsafe::atomic_shared_ptr,
        neatlib::unsafe::shared_ptr> *mhash;
#else
neatlib::ConcurrentHashTable<uint64_t, uint64_t, std::hash<size_t>, 4, 16> *mhash;
#endif
#elif COUNT_HASH == 2
neatlib::BasicHashTable<uint64_t,
        uint64_t,
        std::hash<uint64_t>,
        std::equal_to<uint64_t>,
        std::allocator<std::pair<const uint64_t, uint64_t>>, 4> *mhash;
#elif COUNT_HASH == 3
neatlib::UniversalHashTable<uint64_t, uint64_t, std::hash<size_t>, 4, 16> *mhash;
#else
neatlib::LockFreeHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16> *mhash;
#endif

uint64_t *loads;

long total_time;

uint64_t exists = 0;

uint64_t success = 0;

uint64_t failure = 0;

uint64_t total_count = (1LLU << 20);

uint64_t thread_count = 8;

uint64_t total_round = 8;

stringstream *output;

pthread_t *workers;

int needHit = 0; // 0: expect = i; 1: expect = total- i; 2: notfound.

void *singleInsert(void *args) {
    for (uint64_t i = 0; i < total_count; i++) {
        // mhash->Insert(loads[i], loads[i]);
        mhash->Insert(i, i); // return what when duplication heappens.
    }
}

void *singleDelete(void *args) {
    for (uint64_t i = 0; i < total_count; i++) {
        mhash->Remove(i); // return what when duplication heappens.
    }
}

void *singleUpdate(void *args) {
    for (uint64_t i = 0; i < total_count; i++) {
        mhash->Update(i, total_count - i); // return what when duplication heappens.
    }
}

void *singleRead(void *args) {
    for (uint64_t i = 0; i < total_count / thread_count; i++) {
        pair<uint64_t, uint64_t> ret = mhash->Get(i);
        if (needHit == 0) {
            assert(ret.second == i);
        } else if (needHit == 1) {
            assert(ret.second == (total_count - i));
        } else {

        }
    }
}

int main(int argc, char **argv) {
    if (argc > 3) {
        thread_count = std::atol(argv[1]);
        total_count = std::atol(argv[2]);
        total_round = std::atol(argv[3]);
    }
    cout << " threads: " << thread_count << " count: " << total_count << " round: " << total_round << endl;

    Tracer tracer;
    //tracer.startTime();
#if COUNT_HASH == 1
#ifdef UNSAFE
    mhash = new neatlib::ConcurrentHashTable<uint64_t, uint64_t,
            std::hash<uint64_t>, 4, 16,
            neatlib::unsafe::atomic_shared_ptr,
            neatlib::unsafe::shared_ptr>();
#else
    mhash = new neatlib::ConcurrentHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16>();
#endif
#elif COUNT_HASH == 2
    mhash = new neatlib::BasicHashTable<uint64_t,
            uint64_t,
            std::hash<uint64_t>,
            std::equal_to<uint64_t>,
            std::allocator<std::pair<const uint64_t, uint64_t>>, 4>();
#elif COUNT_HASH == 3
    mhash = new neatlib::UniversalHashTable<uint64_t, uint64_t, std::hash<size_t>, 4, 16>();
#else
    mhash = new neatlib::LockFreeHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16>(thread_number);
#endif
    //cout << "New store: " << tracer.getRunTime() << endl;
    for (uint64_t r = 0; r < total_round; r++) {
        cout << "Round: " << r << endl;
        tracer.startTime();
        cout << "\tBefore insertion" << " elipsed: " << tracer.getRunTime() << endl;
        pthread_t threads[thread_count];
        tracer.startTime();
        for (int i = 0; i < thread_count; i++) {
            pthread_create(&threads[i], nullptr, singleInsert, nullptr);
        }
        for (int i = 0; i < thread_count; i++) {
            pthread_join(threads[i], nullptr);
        }
        cout << "\tAfter insertion" << " elipsed: " << tracer.getRunTime() << endl;
        tracer.startTime();
        needHit = 0;
        for (int i = 0; i < thread_count; i++) {
            pthread_create(&threads[i], nullptr, singleRead, nullptr);
        }
        for (int i = 0; i < thread_count; i++) {
            pthread_join(threads[i], nullptr);
        }
        cout << "\tAfter read " << " elipsed: " << tracer.getRunTime() << endl;
        tracer.startTime();
        for (int i = 0; i < thread_count; i++) {
            pthread_create(&threads[i], nullptr, singleUpdate, nullptr);
        }
        for (int i = 0; i < thread_count; i++) {
            pthread_join(threads[i], nullptr);
        }
        cout << "\tAfter update" << " elipsed: " << tracer.getRunTime() << endl;
        tracer.startTime();
        needHit = 1;
        for (int i = 0; i < thread_count; i++) {
            pthread_create(&threads[i], nullptr, singleRead, nullptr);
        }
        for (int i = 0; i < thread_count; i++) {
            pthread_join(threads[i], nullptr);
        }
        cout << "\tAfter read" << " elipsed: " << tracer.getRunTime() << endl;
        tracer.startTime();
        for (int i = 0; i < thread_count; i++) {
            pthread_create(&threads[i], nullptr, singleDelete, nullptr);
        }
        for (int i = 0; i < thread_count; i++) {
            pthread_join(threads[i], nullptr);
        }
        cout << "\tAfter deletion" << " elipsed: " << tracer.getRunTime() << endl;
        tracer.startTime();
        needHit = 2;
        for (int i = 0; i < thread_count; i++) {
            pthread_create(&threads[i], nullptr, singleRead, nullptr);
        }
        for (int i = 0; i < thread_count; i++) {
            pthread_join(threads[i], nullptr);
        }
        cout << "\tAfter read" << " elipsed: " << tracer.getRunTime() << endl;
    }
    //delete store;
}