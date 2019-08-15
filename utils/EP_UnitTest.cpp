//
// Created by lwh on 19-8-11.
//

#include <functional>
#include <iostream>
#include <stack>
#include <thread>
#include <unordered_set>
#include "tracer.h"
#include "light_epoch.h"
#include "null_disk.h"
#include "malloc_fixed_page_size.h"
#include "universal_hash_table.h"

#if defined(__linux__)

#include <tr1/functional>
#include <cpuid.h>

#else
#define stdHasher boost::hash_detail::hash_value_unsigned<uint64_t>
#endif

using namespace std;
using namespace boost;
using namespace neatlib;
using namespace FASTER::memory;

#define VARIANT_FIELD 1
#define CACHE_RESERVE (1 << 8)
#define INPLACE_NEW   0
#define ROUND 6

uint64_t total = 100000000;
int pdegree = 4;
int simple = 0;
int oneshot = 1;
atomic<long> total_tick(0);
long total_newtime = 0;
long total_freetime = 0;
long max_runtime = 0;
long min_runtime = std::numeric_limits<long>::max();
atomic<int> stopMeasure(0);

struct alignas(32) Item {
    uint8_t buffer[32];
};

void simpleEpoch() {
    typedef UniversalHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16>::data_node datanode;
    typedef MallocFixedPageSize<Item, FASTER::io::NullDisk> alloc_t;
    LightEpoch epoch;
    alloc_t allocator;
    cout << "Epoch page size: " << sizeof(datanode) << endl;
    allocator.Initialize(sizeof(datanode), epoch);
    FixedPageAddress mynode1 = allocator.Allocate();
    cout << "Address: " << mynode1.kPageBits << " " << mynode1.offset() << endl;
    FixedPageAddress mynode2 = allocator.Allocate();
    cout << "Address: " << mynode2.kPageBits << " " << mynode2.offset() << endl;
    FixedPageAddress mynode3 = allocator.Allocate();
    cout << "Address: " << mynode3.kPageBits << " " << mynode3.offset() << endl;
    FixedPageAddress mynode4 = allocator.Allocate();
    cout << "Address: " << mynode4.kPageBits << " " << mynode4.offset() << endl;
    allocator.FreeAtEpoch(mynode1, 0);
    allocator.FreeAtEpoch(mynode2, 1);
    FixedPageAddress mynode5 = allocator.Allocate();
    cout << "Address: " << mynode5.kPageBits << " " << mynode5.offset() << endl;
    FixedPageAddress mynode6 = allocator.Allocate();
    cout << "Address: " << mynode6.kPageBits << " " << mynode6.offset() << endl;
}

void newWorker(bool inBatch, int tid, long *newtime, long *freetime, long *tick) {
    typedef UniversalHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16>::data_node datanode;
    datanode **loads = new datanode *[total];
    Tracer tracer;
    bool run = false;
    tracer.startTime();

    while (stopMeasure.load(memory_order_relaxed) == 0 || !run) {
        run = true;
        if (inBatch) {
            datanode *cache;
            size_t cursor = CACHE_RESERVE;
            std::hash<uint64_t> hasher;
            std::stack<datanode *> allocated;
            for (int i = 0; i < total; i++) {
                if (cursor == CACHE_RESERVE) {
                    cache = (datanode *) malloc(sizeof(datanode) * CACHE_RESERVE);
                    allocated.push(cache);
                    cursor = 0;
                }
#if INPLACE_NEW
                loads[i] = new(cache + cursor)datanode(i, i);
#else
                loads[i] = &cache[cursor];
                loads[i]->reset(i, i);
                uint64_t hashv = loads[i]->hash();
                uint64_t key = loads[i]->get().first;
                uint64_t value = loads[i]->get().second;
#endif
                cursor++;
                (*tick)++;
            }
            *newtime += tracer.getRunTime();
#if INPLACE_NEW
            for (int i = 0; i < total; i++) {
                // De-constructor can not really delete each object here.
                loads[i]->~datanode();
            }
#endif
            while (!allocated.empty()) {
                datanode *element = allocated.top();
                allocated.pop();
                free(element);
            }
            *freetime += tracer.getRunTime();
        } else {
            for (int i = 0; i < total; i++) {
                loads[i] = new datanode(i, i, sizeof(uint64_t));
                (*tick)++;
            }
            *newtime = tracer.getRunTime();
            for (int i = 0; i < total; i++) {
                delete loads[i];
            }
            *freetime += tracer.getRunTime();
        }
    }
    total_tick.fetch_add(*tick, memory_order_release);
    delete[] loads;
}

void concurrentDataAllocate(bool inBatch) {
    cout << "Allocator per " << sizeof(UniversalHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16>::data_node)
         << endl;
    std::vector<std::thread> workers;
    long newtime[pdegree];
    long freetime[pdegree];
    long tick[pdegree];
    total_newtime = 0;
    total_freetime = 0;
    total_tick.store(0, memory_order_release);

    stopMeasure.store(0, memory_order_relaxed);
    Timer timer;
    timer.start();
    for (int t = 0; t < pdegree; t++) {
        newtime[t] = 0;
        freetime[t] = 0;
        tick[t] = 0;
        workers.push_back(std::thread(newWorker, inBatch, t, newtime + t, freetime + t, tick + t));
    }
    if (!oneshot) {
        while (timer.elapsedSeconds() < default_timer_range) {
            sleep(1);
        }
    }
    stopMeasure.store(1, memory_order_relaxed);

    for (int t = 0; t < pdegree; t++) {
        workers[t].join();
        total_newtime += newtime[t];
        total_freetime += freetime[t];
        cout << "\t" << t << " " << newtime[t] << " " << freetime[t] << " " << tick[t] << endl;
    }
}

typedef UniversalHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16>::data_node datanode;
typedef MallocFixedPageSize<Item, FASTER::io::NullDisk> alloc_t;

void eopchWorker(bool inBatch, alloc_t *allocator, int tid, long *newtime, long *freetime, long *tick) {
    FixedPageAddress *loads = new FixedPageAddress[total];
    Tracer tracer;
    bool run = false;
    tracer.startTime();

    while (stopMeasure.load(memory_order_relaxed) == 0 || !run) {
        run = true;
        for (int i = 0; i < total; i++) {
            //loads[i] = new datanode(i, i, sizeof(uint64_t));
            loads[i] = allocator->Allocate();
            (*tick)++;
        }
        *newtime += tracer.getRunTime();
        for (int i = 0; i < total; i++) {
            allocator->FreeAtEpoch(loads[i], 0);
        }
        *freetime += tracer.getRunTime();
    }
    total_tick.fetch_add(*tick, memory_order_release);
    delete[] loads;
}

void concurrentEopchAllocate(bool inBatch) {
    cout << "Allocator per " << sizeof(datanode)
         << endl;
    std::vector<std::thread> workers;
    LightEpoch epoch;
    alloc_t allocator;
    allocator.Initialize(sizeof(datanode), epoch);
    total_newtime = 0;
    total_freetime = 0;
    long newtime[pdegree];
    long freetime[pdegree];
    long tick[pdegree];
    total_tick.store(0, memory_order_release);

    stopMeasure.store(0, memory_order_relaxed);
    Timer timer;
    timer.start();
    for (int t = 0; t < pdegree; t++) {
        newtime[t] = 0;
        freetime[t] = 0;
        tick[t] = 0;
        workers.push_back(std::thread(eopchWorker, inBatch, &allocator, t, newtime + t, freetime + t, tick + t));
    }
    if (!oneshot) {
        while (timer.elapsedSeconds() < default_timer_range) {
            sleep(1);
        }
    }
    stopMeasure.store(1, memory_order_relaxed);

    for (int t = 0; t < pdegree; t++) {
        workers[t].join();
        total_newtime += newtime[t];
        total_freetime += freetime[t];
        cout << "\t" << t << " " << newtime[t] << " " << freetime[t] << " " << tick[t] << endl;
    }
}

int main(int argc, char **argv) {
    if (argc > 4) {
        total = std::atol(argv[1]);
        pdegree = std::atoi(argv[2]);
        simple = std::atoi(argv[3]);
        oneshot = std::atoi(argv[4]);
    }
    cout << total << " " << pdegree << " " << simple << " " << oneshot << endl;
    Tracer tracer;
    if (simple) {
        tracer.startTime();
        simpleEpoch();
        cout << "Ist: " << tracer.getRunTime() << endl;
    } else {
        for (int i = 0; i <= ROUND; i++) {
            cout << "Round: " << i << endl;
            tracer.startTime();
            concurrentDataAllocate(false);
            cout << "new: " << tracer.getRunTime() << " " << total_tick.load(memory_order_release) << " tpt: "
                 << (double) total_tick.load(memory_order_relaxed) * pdegree / (total_newtime + total_freetime) << endl;
            tracer.startTime();
            concurrentDataAllocate(true);
            cout << "bew: " << tracer.getRunTime() << " " << total_tick.load(memory_order_release) << " tpt: "
                 << (double) total_tick.load(memory_order_relaxed) * pdegree / (total_newtime + total_freetime) << endl;
            tracer.startTime();
            concurrentEopchAllocate(true);
            cout << "eew: " << tracer.getRunTime() << " " << total_tick.load(memory_order_release) << " tpt: "
                 << (double) total_tick.load(memory_order_relaxed) * pdegree / (total_newtime + total_freetime) << endl;
            tracer.startTime();
            simpleEpoch();
            cout << "Ist: " << tracer.getRunTime() << endl;
        }
    }
    return 0;
}