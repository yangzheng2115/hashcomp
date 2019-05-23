#include <iostream>
#include <sstream>
#include "tracer.h"
#include <stdio.h>
#include <stdlib.h>
#include "faster.h"
#include "kvcontext.h"

#define DEFAULT_HASH_LEVEL (25)
#define DEFAULT_THREAD_NUM (8)
#define DEFAULT_KEYS_COUNT (1 << 20)
#define DEFAULT_KEYS_RANGE (1 << 2)

#define DEFAULT_STR_LENGTH 256
#define DEFAULT_KEY_LENGTH 8

#define TEST_LOOKUP        1

#define DEFAULT_STORE_BASE 100000000

using namespace FASTER::api;

#ifdef _WIN32
typedef hreadPoolIoHandler handler_t;
#else
typedef QueueIoHandler handler_t;
#endif
typedef FileSystemDisk<handler_t, 1073741824ull> disk_t;

using store_t = FasterKv<Key, Value, disk_t>;

size_t init_size = next_power_of_two(DEFAULT_STORE_BASE / 2);

store_t store{init_size, 17179869184, "storage"};

uint64_t *loads;

long total_time;

uint64_t exists = 0;

uint64_t success = 0;

uint64_t failure = 0;

uint64_t total_count = DEFAULT_KEYS_COUNT;

int thread_number = DEFAULT_THREAD_NUM;

int key_range = DEFAULT_KEYS_RANGE;

stringstream *output;

atomic<int> stopMeasure(0);

struct target {
    int tid;
    uint64_t *insert;
    store_t *store;
};

pthread_t *workers;

struct target *parms;

void simpleInsert() {
    Tracer tracer;
    tracer.startTime();
    int inserted = 0;
    for (int i = 0; i < total_count; i++) {
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<UpsertContext> context{ctxt};
        };
        UpsertContext context{loads[i], loads[i]};
        Status stat = store.Upsert(context, callback, 1);
        inserted++;
    }
    cout << inserted << " " << tracer.getRunTime() << endl;
}

void *insertWorker(void *args) {
    //struct target *work = (struct target *) args;
    uint64_t inserted = 0;
    Tracer tracer;
    tracer.startTime();
    for (int i = 0; i < total_count / thread_number; i++) {
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<UpsertContext> context{ctxt};
        };
        UpsertContext context{loads[i], loads[i]};
        Status stat = store.Upsert(context, callback, 1);
        inserted++;
    }
    long elipsed = tracer.getRunTime();
    //__sync_fetch_and_add(&exists, fail);
    __sync_fetch_and_add(&total_time, elipsed);
    __sync_fetch_and_add(&failure, inserted);
}

void *measureWorker(void *args) {
//    Tracer tracer;
//    tracer.startTime();
//    struct target *work = (struct target *) args;
//    uint64_t hit = 0;
//    uint64_t fail = 0;
//    uint8_t value[DEFAULT_KEY_LENGTH];
//    while (stopMeasure.load(memory_order_relaxed) == 0) {
//        for (int i = work->tid; i < total_count; i += thread_number) {
//#if TEST_LOOKUP
//            if (0 == level_query(work->levelHash, &loads[i * DEFAULT_KEY_LENGTH], value)) {
//                hit++;
//            } else {
//                fail++;
//            }
//#else
//            if (0 == level_update(work->levelHash, &loads[i * DEFAULT_KEY_LENGTH], &loads[i * DEFAULT_KEY_LENGTH])) {
//                hit++;
//            } else {
//                fail++;
//            }
//#endif
//        }
//    }
//
//    long elipsed = tracer.getRunTime();
//    output[work->tid] << work->tid << " " << elipsed << endl;
//    __sync_fetch_and_add(&total_time, elipsed);
//    __sync_fetch_and_add(&success, hit);
//    __sync_fetch_and_add(&failure, fail);
}

void prepare() {
    cout << "prepare" << endl;
    workers = new pthread_t[thread_number];
    parms = new struct target[thread_number];
    output = new stringstream[thread_number];
    for (int i = 0; i < thread_number; i++) {
        parms[i].tid = i;
        parms[i].store = &store;
        parms[i].insert = (uint64_t *) calloc(total_count / thread_number, sizeof(uint64_t *));
        char buf[DEFAULT_STR_LENGTH];
        for (int j = 0; j < total_count / thread_number; j++) {
            std::sprintf(buf, "%d", i + j * thread_number);
            parms[i].insert[j] = j;
        }
    }
}

void finish() {
    cout << "finish" << endl;
    for (int i = 0; i < thread_number; i++) {
        delete[] parms[i].insert;
    }
    delete[] parms;
    delete[] workers;
    delete[] output;
}

void multiWorkers() {
    output = new stringstream[thread_number];
    Tracer tracer;
    tracer.startTime();
    for (int i = 0; i < thread_number; i++) {
        pthread_create(&workers[i], nullptr, insertWorker, &parms[i]);
    }
    for (int i = 0; i < thread_number; i++) {
        pthread_join(workers[i], nullptr);
    }
    cout << "Insert " << exists << " " << tracer.getRunTime() << endl;
    Timer timer;
    timer.start();
    for (int i = 0; i < thread_number; i++) {
        pthread_create(&workers[i], nullptr, measureWorker, &parms[i]);
    }
    while (timer.elapsedSeconds() < default_timer_range) {
        sleep(1);
    }
    stopMeasure.store(1, memory_order_relaxed);
    for (int i = 0; i < thread_number; i++) {
        pthread_join(workers[i], nullptr);
        string outstr = output[i].str();
        cout << outstr;
    }
    cout << "Gathering ..." << endl;
}

int main(int argc, char **argv) {
    if (argc > 3) {
        thread_number = std::atol(argv[1]);
        key_range = std::atol(argv[2]);
        total_count = std::atol(argv[3]);
    }
    cout << " threads: " << thread_number << " range: " << key_range << " count: " << total_count << endl;
    loads = (uint64_t *) calloc(total_count, sizeof(uint64_t));
    UniformGen<uint64_t>::generate(loads, key_range, total_count);
    prepare();
    cout << "simple" << endl;
    simpleInsert();
    cout << "multiinsert" << endl;
    multiWorkers();
    cout << "operations: " << success << " failure: " << failure << " throughput: "
         << (double) (success + failure) * thread_number / total_time << endl;
    free(loads);
    finish();
    return 0;
}