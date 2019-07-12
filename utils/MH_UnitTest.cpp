//
// Created by lwh on 19-7-12.
//

#include <iostream>
#include <algorithm>
#include <functional>
#include "tracer.h"
#include "lock_free_hash_table.h"
#include "concurrent_hash_table.h"
#include "basic_hash_table.h"
#include "atomic_shared_ptr.h"

#if defined(__linux__)

#include <cpuid.h>

#endif

uint64_t total = 10000;
int pdegree = 2;

#define testingFixed 0
#if testingFixed
#define type int
#else
#define type char*
type*sinput;
#endif

#define testingType  1 // 0: basic; 1: nhash; 2: chash; 3: lhash.
#define kvLength     4

using equalTo = equal_to<type>;
#if (testingType == 0)
neatlib::BasicHashTable<type, type, std::hash<type>, std::equal_to<type>,
        std::allocator<std::pair<const type, type>>, 4> *store;
#elif (testingType == 1)
neatlib::ConcurrentHashTable<type, type, std::hash<type>, 4, 4,
        neatlib::unsafe::atomic_shared_ptr, neatlib::unsafe::shared_ptr> *store;
#elif (testingType == 2)
neatlib::ConcurrentHashTable<type, type, std::hash<type>, 4, 4> *store;
#else if (testingType == 3)
neatlib::LockFreeHashTable<type, type, std::hash<type>, 4, 4> *store;
#endif

struct paramstruct {
    int tid;
};

void *worker(void *args) {
    paramstruct *param = static_cast<paramstruct *>(args);
    for (int i = param->tid; i < total; i += pdegree) {
#if testingFixed
        store->Insert(i, i);
#else
        store->Insert(sinput[i], sinput[i]);
#endif
    }
}

void insert() {
    pthread_t threads[pdegree];
    paramstruct params[pdegree];
    for (int i = 0; i < pdegree; i++) {
        params[i].tid = i;
        pthread_create(&threads[i], nullptr, worker, &params[i]);
    }
    for (int i = 0; i < pdegree; i++) {
        pthread_join(threads[i], nullptr);
    }
}

void verify() {
    size_t right = 0;
    size_t wrong = 0;
    for (int i = 0; i < total; i++) {
#if testingFixed
        pair<type, type> p = store->Get(i);
        assert(p.first == i);
#else
        pair<type, type> p = store->Get(sinput[i]);
        //equalTo(p.first, sinput[i]);
        if (strcmp(p.first, sinput[i]) == 0) {
            right++;
        } else {
            wrong++;
        }
#endif
    }
    cout << "Right: " << right << " wrong: " << wrong << endl;
}

int main(int argc, char **argv) {
#if defined(__linux__)
    if (sysconf(_SC_NPROCESSORS_ONLN) > 8) {
        total = 100000000;
        pdegree = 32;
    }
#endif
#if !testingFixed
    sinput = new type[total];
    for (int i = 0; i < total; i++) {
        sinput[i] = new char[kvLength];
        memset(sinput[i], static_cast<char>(i), kvLength - 1);
    }
#endif
    cout << total << "/" << pdegree << endl;
#if (testingType == 0)
    store = new neatlib::BasicHashTable<type, type, std::hash<type>, std::equal_to<type>,
            std::allocator<std::pair<const type, type>>, 4>();
#elif (testingType == 1)
    store = new neatlib::ConcurrentHashTable<type, type, std::hash<type>, 4, 4,
            neatlib::unsafe::atomic_shared_ptr, neatlib::unsafe::shared_ptr>();
#elif (testingType == 2)
    store = new neatlib::ConcurrentHashTable<type, type, std::hash<type>, 4, 4>();
#elif (testingType == 3)
    store = new neatlib::LockFreeHashTable<type, type, std::hash<type>, 4, 4>(pdegree);
#endif
    Tracer tracer;
    tracer.startTime();
    insert();
    cout << "Insert: " << tracer.getRunTime() << endl;
    tracer.startTime();
    verify();
    cout << "Verify: " << tracer.getRunTime() << endl;
    return 0;
}