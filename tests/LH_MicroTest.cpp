#include <iostream>
#include "tracer.h"
#include <stdio.h>
#include <stdlib.h>
#include "level_hashing.h"

#define DEFAULT_HASH_LEVEL (25)
#define DEFAULT_THREAD_NUM (8)
#define DEFAULT_KEYS_COUNT (1 << 20)
#define DEFAULT_KEYS_RANGE (1 << 2)

#define DEFAULT_KEY_LENGTH 8

uint8_t *loads;

long total_time;

uint64_t exists = 0;

uint64_t update = 0;

uint64_t failure = 0;

uint64_t total_count = DEFAULT_KEYS_COUNT;

int thread_number = DEFAULT_THREAD_NUM;

int key_range = DEFAULT_KEYS_RANGE;

level_hash *levelHash;

stringstream *output;

atomic<int> stopMeasure(0);

struct target {
    int tid;
    uint8_t **insert;
    level_hash *levelHash;
};

pthread_t *workers;

struct target *parms;

using namespace std;

void prepare() {
    cout << "prepare" << endl;
    workers = new pthread_t[thread_number];
    parms = new struct target[thread_number];
    for (int i = 0; i < thread_number; i++) {
        parms[i].tid = i;
        parms[i].levelHash = levelHash;
        parms[i].insert = (uint8_t **) calloc(total_count / thread_number, sizeof(uint8_t *));
        char buf[DEFAULT_KEY_LENGTH];
        for (int j = 0; j < total_count / thread_number; j++) {
            std::sprintf(buf, "%d", i + j * thread_number);
            parms[i].insert[j] = new uint8_t[DEFAULT_KEY_LENGTH];
            memcpy(parms[i].insert[j], buf, DEFAULT_KEY_LENGTH - 1);
        }
    }
}

void restart() {
    cout << "destroy" << endl;
    level_destroy(levelHash);
    cout << "reinit" << endl;
    levelHash = level_init(DEFAULT_HASH_LEVEL);
    for (int i = 0; i < thread_number; i++) {
        parms[i].levelHash = levelHash;
    }
}

void finish() {
    cout << "finish" << endl;
    for (int i = 0; i < thread_number; i++) {
        for (int j = 0; j < total_count / thread_number; j++) {
            delete[] parms[i].insert[j];
        }
        delete[] parms[i].insert;
    }
}

void simpleInsert() {
    Tracer tracer;
    tracer.startTime();
    int inserted = 0;
    for (int i = 0; i < total_count; i++) {
        if (!level_insert(levelHash, &loads[i * DEFAULT_KEY_LENGTH], &loads[i * DEFAULT_KEY_LENGTH])) {
            inserted++;
        }
    }
    cout << inserted << " " << tracer.getRunTime() << endl;
    tracer.startTime();
    uint64_t found = 0;
    uint8_t value[DEFAULT_KEY_LENGTH];
    for (int i = 0; i < total_count; i += DEFAULT_KEY_LENGTH) {
        found += (0 == level_query(levelHash, &loads[i * DEFAULT_KEY_LENGTH], value));
    }
    cout << found << " " << tracer.getRunTime() << endl;
    tracer.startTime();
    found = 0;
    for (int i = 0; i < total_count; i += DEFAULT_KEY_LENGTH) {
        found += (0 == level_delete(levelHash, &loads[i * DEFAULT_KEY_LENGTH]));
    }
    cout << found << " " << tracer.getRunTime() << endl;
}

void *insertWorker(void *args) {
    struct target *work = (struct target *) args;
    uint64_t fail = 0;
    for (int i = 0; i < total_count / thread_number; i++) {
        if (!level_insert(work->levelHash, work->insert[i], work->insert[i])) {
            fail++;
        }
    }
    __sync_fetch_and_add(&exists, fail);
}

/*void *measureWorker(void *args) {
    Tracer tracer;
    tracer.startTime();
    struct target *work = (struct target *) args;
    //cout << "Updater " << work->tid << endl;
    uint64_t hit = 0;
    uint64_t fail = 0;
    while (stopMeasure.load(memory_order_relaxed) == 0) {
        for (int i = work->tid; i < total_count; i += thread_number) {
#if TEST_LOOKUP
            if (work->set->contains(loads[i])) {
                hit++;
            } else {
                fail++;
            }
#else
            if (work->set->remove(loads[i])) {
                hit++;
                if (!work->set->add(loads[i])) {
                    fail++;
                }
            }
#endif
        }
    }

    long elipsed = tracer.getRunTime();
    output[work->tid] << work->tid << " " << elipsed << endl;
    __sync_fetch_and_add(&total_time, elipsed);
    __sync_fetch_and_add(&update, hit);
    __sync_fetch_and_add(&failure, fail);
}*/

void multiWorkers() {
    Tracer tracer;
    tracer.startTime();
    for (int i = 0; i < thread_number; i++) {
        pthread_create(&workers[i], nullptr, insertWorker, &parms[i]);
    }
    for (int i = 0; i < thread_number; i++) {
        pthread_join(workers[i], nullptr);
    }
    cout << "Insert " << exists << " " << tracer.getRunTime() << endl;
    /*
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
    }*/
    cout << "Gathering ..." << endl;
    finish();
    delete[] parms;
    delete[] workers;
}

int main(int argc, char **argv) {
    if (argc > 3) {
        thread_number = std::atol(argv[1]);
        key_range = std::atol(argv[2]);
        total_count = std::atol(argv[3]);
    }
    cout << " threads: " << thread_number << " range: " << key_range << " count: " << total_count << endl;
    loads = (uint8_t *) calloc(DEFAULT_KEY_LENGTH * total_count, sizeof(uint8_t));
    UniformGen<uint8_t>::generate(loads, DEFAULT_KEY_LENGTH, key_range, total_count);
    levelHash = level_init(DEFAULT_HASH_LEVEL);
    simpleInsert();
    prepare();
    cout << "restart" << endl;
    restart();
    cout << "multiinsert" << endl;
    multiWorkers();
    free(loads);
    return 0;
}