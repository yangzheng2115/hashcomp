#include <iostream>
#include <sstream>
#include "tracer.h"
#include <stdio.h>
#include <stdlib.h>
#include "faster.h"
#include "kvcontext.h"

#define DEFAULT_THREAD_NUM (8)
#define DEFAULT_KEYS_COUNT (1 << 20)
#define DEFAULT_KEYS_RANGE (1 << 20)

#define DEFAULT_STR_LENGTH 256
//#define DEFAULT_KEY_LENGTH 8

#define ITERATION_MODE      0
#define PERMUTATE_MODE      1
#define REPLICATE_MODE      2

#define DEFAULT_STORE_BASE 10000LLU

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

int operatemode = ITERATION_MODE;

uint64_t *loads;

long total_time;

long insert_time;

int iter_round = 0;

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
    bool type;
    store_t *store;
};

pthread_t *workers;

struct target *parms;

void prepare() {
    cout << "prepare" << endl;
    workers = new pthread_t[thread_number];
    parms = new struct target[thread_number];
    output = new stringstream[thread_number];
    for (int i = 0; i < thread_number; i++) {
        parms[i].tid = i;
        parms[i].type = true;
        parms[i].store = &store;
        char buf[DEFAULT_STR_LENGTH];
    }
}

void finish() {
    cout << "finish" << endl;
    delete[] parms;
    delete[] workers;
}

void *operateWorker(void *args) {
    Tracer tracer;
    tracer.startTime();
    struct target *work = (struct target *) args;
    uint64_t hit = 0;
    uint64_t fail = 0;
    store.StartSession();
    for (int i = 0; i < total_count; i++) {
        int idx = -1;
        switch (operatemode) {
            case ITERATION_MODE:
                idx = work->tid * total_count / thread_number + i % (total_count / thread_number);
                break;
            case PERMUTATE_MODE:
                idx = (work->tid + i * total_count / thread_number) % total_count;
                break;
            case REPLICATE_MODE:
                idx = i;
                break;
            default:
                break;
        }
        if (idx == -1) {
            break;
        }
        if (work->type) {
            auto callback = [](IAsyncContext *ctxt, Status result) {
                CallbackContext<UpsertContext> context{ctxt};
            };
            UpsertContext context{loads[idx], loads[idx]};
            Status stat = store.Upsert(context, callback, 1);
            if (stat == Status::NotFound)
                hit++;
            else
                fail++;
        } else {
            auto callback = [](IAsyncContext *ctxt, Status result) {
                CallbackContext<DeleteContext> context(ctxt);
            };
            DeleteContext context{loads[idx]};
            store.Delete(loads[i], context, 1);
        }
    }

    long elipsed = tracer.getRunTime();
    output[work->tid] << "\t" << work->tid << " " << iter_round << " " << elipsed << " " << hit << endl;
    if (work->type) {
        __sync_fetch_and_add(&insert_time, elipsed);
    }
    __sync_fetch_and_add(&total_time, elipsed);
    __sync_fetch_and_add(&success, hit);
    __sync_fetch_and_add(&failure, fail);
    store.StopSession();
}

void scheduleWorkers() {
    Tracer tracer;
    tracer.startTime();
    while (stopMeasure.load(memory_order_relaxed) == 0) {
        for (int i = 0; i < thread_number; i++) {
            parms[i].type != parms[i].type;
            pthread_create(&workers[i], nullptr, operateWorker, &parms[i]);
        }
        for (int i = 0; i < thread_number; i++) {
            pthread_join(workers[i], nullptr);
        }
        iter_round++;
    }
}

void operateWorkers() {
    output = new stringstream[thread_number];
    insert_time = 0;
    total_time = 0;
    success = 0;
    failure = 0;
    Tracer tracer;
    tracer.startTime();
    stopMeasure.store(0, memory_order_relaxed);
    Timer timer;
    timer.start();
    store.StartSession();
    thread t = thread(scheduleWorkers);
    while (timer.elapsedSeconds() < default_timer_range) {
        sleep(1);
    }
    stopMeasure.store(1, memory_order_relaxed);
    t.join();
    store.StopSession();
    cout << "\tTill SessionStop " << tracer.fetchTime() << endl;
    cout << "Gathering ..." << endl;
    for (int t = 0; t < thread_number; t++) {
        string outstr = output[t].str();
        cout << outstr;
    }
    cout << "\tRound 0: " << store.Size() << " " << tracer.getRunTime() << endl;
    cout << "operations: " << success << " failure: " << failure << " throughput: "
         << (double) (success + failure) * thread_number / total_time << endl;
    delete[] output;
}

void resizeStore() {
    Tracer tracer;
    for (int d = 1; init_size <= total_count * 2; init_size *= 2, d++) {
        tracer.startTime();
        store.StartSession();
        cout << "\t\tTill SessionStart " << tracer.fetchTime() << endl;
        static std::atomic<bool> grow_done{false};
        auto callback = [](uint64_t new_size) {
            grow_done = true;
        };
        store.GrowIndex(callback);
        while (!grow_done) {
            store.Refresh();
            std::this_thread::yield();
        }
        cout << "\t\tTill SessionStop " << tracer.fetchTime() << endl;
        store.StopSession();
        cout << "\tRound " << d << ": " << store.Size() << " " << tracer.getRunTime() << endl;
    }
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
    cout << "operations: " << success << " failure: " << failure << " throughput: "
         << (double) (success + failure) * thread_number / total_time << endl;
    for (int r = 0; r < 3; r++) {
        operateWorkers();
    }
    resizeStore();
    for (int r = 0; r < 3; r++) {
        operateWorkers();
    }
    free(loads);
    finish();
    return 0;
}