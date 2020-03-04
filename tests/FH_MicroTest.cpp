#include <iostream>
#include <sstream>
#include <fstream>
#include <experimental/filesystem>
#include "tracer.h"
#include <stdio.h>
#include <stdlib.h>
#include "faster.h"
#include "address.h"
#include "../io/file_system_disk.h"
#include "../io/null_disk.h"

#define CONTEXT_TYPE 0
#if CONTEXT_TYPE == 0

#include "kvcontext.h"

#elif CONTEXT_TYPE == 2

#include "cvkvcontext.h"

#endif

#define DEFAULT_THREAD_NUM (4)
#define DEFAULT_KEYS_COUNT (1 << 23)
#define DEFAULT_KEYS_RANGE (1 << 30)

#define DEFAULT_STR_LENGTH 256
//#define DEFAULT_KEY_LENGTH 8

#define TEST_LOOKUP        0

#define DEFAULT_STORE_BASE 100000000LLU

using namespace FASTER::api;
using namespace FASTER::io;

#ifdef _WIN32
typedef hreadPoolIoHandler handler_t;
#else
typedef QueueIoHandler handler_t;
#endif
typedef FileSystemDisk<handler_t, 1073741824ull> disk_t;

using store_t = FasterKv<Key, Value, disk_t>;
//using store_t = FasterKv<Key, Value, FASTER::io::NullDisk>;
size_t init_size = next_power_of_two(DEFAULT_STORE_BASE / 2);

store_t store{init_size, 17179869184, "storage"};
//FasterKv<Key, Value, FASTER::io::NullDisk> store{128, 1073741824, ""};
uint64_t *loads;

#if CONTEXT_TYPE == 2
uint64_t *content;
#endif

long total_time;

uint64_t exists = 0;

uint64_t success = 0;

uint64_t failure = 0;

//uint64_t total_count = DEFAULT_KEYS_COUNT;
uint64_t total_count = 20000000;

uint64_t timer_range = default_timer_range;

uint64_t kCheckpointInterval = 1 << 20;
uint64_t kRefreshInterval = 1 << 8;
uint64_t kCompletePendingInterval = 1 << 12;
Guid retoken;
int cd = 0;
int rounds = 0;
int thread_number = DEFAULT_THREAD_NUM;

//int key_range = DEFAULT_KEYS_RANGE;
int key_range = 1000000000;

stringstream *output;

atomic<int> stopMeasure(0);

struct target {
    int tid;
    uint64_t *insert;
    store_t *store;
};

pthread_t *workers;

struct target *parms;

void simpleInsert1() {
    Tracer tracer;
    tracer.startTime();
    int inserted = 0;
    int j = 0;
    auto hybrid_log_persistence_callback = [](Status result, uint64_t persistent_serial_num) {
        if (result != Status::Ok) {
            printf("Thread %" PRIu32 " reports checkpoint failed.\n",
                   Thread::id());
        } else {
            printf("Thread %" PRIu32 " reports persistence until %" PRIu64 "\n",
                   Thread::id(), persistent_serial_num);
        }
    };
    //store.StartSession();
    for (uint64_t i = 0; i < total_count; i++) {
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<UpsertContext> context{ctxt};
        };
#if CONTEXT_TYPE == 0
        UpsertContext context{i, i};
#elif CONTEXT_TYPE == 2
        UpsertContext context(loads[i], 8);
        context.reset((uint8_t *) (content + i));
#endif
        Status stat = store.Upsert(context, callback, 1);
        inserted++;
/*
            if (i % kCompletePendingInterval == 0) {
                store.CompletePending(false);
            } else if (i % kRefreshInterval == 0) {
                store.Refresh();
            }
*/
    }

    //store.CompletePending(true);
    // Deregister thread from FASTER
    //store.StopSession();
    cout << inserted << " " << tracer.getRunTime() << endl;
}

void simpleRead() {
    uint64_t hit = 0;
    uint64_t fail = 0;
    for (uint64_t i = 0; i < total_count; i++) {
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<ReadContext> context{ctxt};
        };
        ReadContext context{i};
        if(i==7931739)
            int s=0;
        Status result = store.Read(context, callback, 1);
            if (result == Status::Ok)
            hit++;
        else
            fail++;
    }
    cout << hit << " " << fail << endl;
}

void simpleInsert() {
    Tracer tracer;
    tracer.startTime();
    int inserted = 0;
    int j = 0;
    auto hybrid_log_persistence_callback = [](Status result, uint64_t persistent_serial_num) {
        if (result != Status::Ok) {
            printf("Thread %" PRIu32 " reports checkpoint failed.\n",
                   Thread::id());
        } else {
            printf("Thread %" PRIu32 " reports persistence until %" PRIu64 "\n",
                   Thread::id(), persistent_serial_num);
        }
    };
    store.StartSession();
    for (int i = 0; i < total_count; i++) {
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<UpsertContext> context{ctxt};
        };
#if CONTEXT_TYPE == 0
        UpsertContext context{loads[i], loads[i]};
#elif CONTEXT_TYPE == 2
        UpsertContext context(loads[i], 8);
        context.reset((uint8_t *) (content + i));
#endif
        Status stat = store.Upsert(context, callback, 1);
        inserted++;

        if (i % kCheckpointInterval == 0 && i != 0 && j == 0) {
            Guid token;
            cout << "checkpoint start in" << i << endl;
            if (store.Checkpoint(nullptr, hybrid_log_persistence_callback, token))
                //if(store.CheckpointIndex(nullptr,token))
                //if (store.CheckpointHybridLog(hybrid_log_persistence_callback, token))
                //if(store.CheckpointIndex(nullptr,token))
            {
                if (j == 0)
                    printf("Calling Checkpoint(), token = %s\n", token.ToString().c_str());
                j++;
            }
        }
        if (i % kCompletePendingInterval == 0) {
            store.CompletePending(false);
        } else if (i % kRefreshInterval == 0) {
            store.Refresh();
            if (j == 1 && store.CheckpointCheck()) {
                cout << i << endl;
                cd = i;
                break;
            }
        }

    }
    //store.CompletePending(true);
    // Deregister thread from FASTER
    store.StopSession();
    cout << inserted << " " << tracer.getRunTime() << endl;
}

void RecoverAndTest(const Guid &index_token, const Guid &hybrid_log_token) {
    uint32_t version;
    uint64_t hit = 0;
    uint64_t fail = 0;
    std::vector<Guid> session_ids;
    store.Recover(index_token, hybrid_log_token, version, session_ids);
    cout << "recover successful" << endl;
    for (uint64_t i = 0; i < total_count*2; i++) {
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<ReadContext> context{ctxt};
        };
#if CONTEXT_TYPE == 0
        ReadContext context{i};
        Status result = store.Read(context, callback, 1);
        if (result == Status::Ok)
            hit++;
        else
            fail++;
#elif CONTEXT_TYPE == 2
        ReadContext context(loads[i]);

        Status result = store.Read(context, callback, 1);
        if (result == Status::Ok && *(uint64_t *) (context.output_bytes) == total_count - loads[i])
            hit++;
        else
            fail++;
#endif
    }
    cout << hit << "  " << fail << endl;
}

void Populate() {
    auto hybrid_log_persistence_callback = [](Status result, uint64_t persistent_serial_num) {
        if (result != Status::Ok) {
            printf("Thread %" PRIu32 " reports checkpoint failed.\n",
                   Thread::id());
        } else {
            printf("Thread %" PRIu32 " reports persistence until %" PRIu64 "\n",
                   Thread::id(), persistent_serial_num);
        }
    };

    // Register thread with FASTER
    store.StartSession();

    // Process the batch of input data
    for (uint64_t idx = 0; idx < total_count; ++idx) {
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<UpsertContext> context{ctxt};
        };
#if CONTEXT_TYPE == 0
        UpsertContext context{loads[idx], loads[idx]};
#elif CONTEXT_TYPE == 2
        UpsertContext context(loads[idx], 8);
        context.reset((uint8_t *) (content + idx));
#endif

        if (idx % kCheckpointInterval == 0) {
            Guid token;
            // if (store.Checkpoint(nullptr, hybrid_log_persistence_callback, token))
            //printf("Calling Checkpoint(), token = %s\n", token.ToString().c_str());
        }
    }
    // Make sure operations are completed
    store.CompletePending(true);

    // Deregister thread from FASTER
    store.StopSession();

    printf("Populate successful\n");
}

void *insertWorker(void *args) {
    //struct target *work = (struct target *) args;
    uint64_t inserted = 0;
    struct target *work = (struct target *) args;
    int k = work->tid;
    for (int i = k * total_count / thread_number; i < (k + 1) * total_count / thread_number; i++) {
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<UpsertContext> context{ctxt};
        };
#if CONTEXT_TYPE == 0
        UpsertContext context{loads[i], loads[i]};
#elif CONTEXT_TYPE == 2
        UpsertContext context(loads[i], 8);
        context.reset((uint8_t *) (content + i));
#endif
        Status stat = store.Upsert(context, callback, 1);
        inserted++;
    }
    __sync_fetch_and_add(&exists, inserted);
}

void *checkWorker(void *args) {
    int inserted = 0;
    int j = 0;
    struct target *work = (struct target *) args;
    int k = work->tid;
    auto hybrid_log_persistence_callback = [](Status result, uint64_t persistent_serial_num) {
        if (result != Status::Ok) {
            printf("Thread %" PRIu32 " reports checkpoint failed.\n",
                   Thread::id());
        } else {
            printf("Thread %" PRIu32 " reports persistence until %" PRIu64 "\n",
                   Thread::id(), persistent_serial_num);
        }
    };
    store.StartSession();
    s:
    for (uint64_t  i = total_count; i < total_count*2; i++) {
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<UpsertContext> context{ctxt};
        };
#if CONTEXT_TYPE == 0
        UpsertContext context{i, i};
#elif CONTEXT_TYPE == 2
        UpsertContext context(loads[i], 8);
        context.reset((uint8_t *) (content + i));
#endif
        store.Refresh1();
        //Status stat = store.Upsert(context, callback, i);
        Status stat = store.UpsertT(context, callback, i,thread_number);
        inserted++;
        int excepcted = 0;
        if (i % kCheckpointInterval == 0 && i != 0 && stopMeasure.compare_exchange_strong(excepcted, 1)) {
            Guid token;
            cout << "checkpoint start in" << i << endl;
            //if (store.CheckpointHybridLog(hybrid_log_persistence_callback, token))
            if (store.Checkpoint(nullptr, hybrid_log_persistence_callback, token))
            // if(store.CheckpointIndex(nullptr, token))
            {
                printf("Calling Checkpoint(), token = %s\n", token.ToString().c_str());
                cout << "thread id" << k << endl;
            }
        }
        if (i % kCompletePendingInterval == 0) {
            store.CompletePending(false);
        } else if (i % kRefreshInterval == 0) {
            store.Refresh2();
            if (stopMeasure.load() == 1 && store.CheckpointCheck()) {
                //cout <<"thread id:"<<k<<" end in"<< i << endl;
                j++;
                if (j == 2) {
                    j = i;
                    break;
                }

            }
        }
    }
    if (!store.CheckpointCheck()) {
        cout << work->tid << "fail" << endl;
        //goto s;
    }
    //store.CompletePending(true);
    store.StopSession();
    //output[work->tid] << work->tid << " " << j << endl;
}

void *gcWorker(void *args) {
    int inserted = 0;
    int j = 0;
    struct target *work = (struct target *) args;
    int k = work->tid;
    store.StartSession();
    s:
    for (uint64_t  i = total_count; i < total_count*2; i++) {
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<UpsertContext> context{ctxt};
        };
#if CONTEXT_TYPE == 0
        UpsertContext context{i, i};
#elif CONTEXT_TYPE == 2
        UpsertContext context(loads[i], 8);
        context.reset((uint8_t *) (content + i));
#endif
        store.Refresh1();
        //Status stat = store.Upsert(context, callback, i);
        Status stat = store.UpsertT(context, callback, i,thread_number);
        inserted++;
        int excepcted = 0;
        if (stopMeasure.compare_exchange_strong(excepcted, 1)) {
            Address a;
            //if (store.CheckpointHybridLog(hybrid_log_persistence_callback, token))
            if(store.GrabageCollecton(a, nullptr, nullptr)){
                cout<<"garbage collection begin"<<endl;
            }
        }
        if (i % kCompletePendingInterval == 0) {
            store.CompletePending(false);
        } else if (i % kRefreshInterval == 0) {
            store.Refresh2();
        }
    }
    //store.CompletePending(true);
    store.StopSession();
    //output[work->tid] << work->tid << " " << j << endl;
}

void multiPoints() {
    output = new stringstream[thread_number];
    for (int i = 0; i < thread_number; i++) {
        pthread_create(&workers[i], nullptr, checkWorker, &parms[i]);
    }
    for (int i = 0; i < thread_number; i++) {
        pthread_join(workers[i], nullptr);
       // string outstr = output[i].str();
        //cout << outstr;
    }
    cout << "checkpoint done ..." << endl;
}

void GC() {
    output = new stringstream[thread_number];
    for (int i = 0; i < thread_number; i++) {
        pthread_create(&workers[i], nullptr, gcWorker, &parms[i]);
    }
    for (int i = 0; i < thread_number; i++) {
        pthread_join(workers[i], nullptr);
        // string outstr = output[i].str();
        //cout << outstr;
    }
    cout << "garbage collection done ..." << endl;
}

void *measureWorker(void *args) {
    Tracer tracer;
    tracer.startTime();
    struct target *work = (struct target *) args;
    uint64_t hit = 0;
    uint64_t fail = 0;
    long elipsed;
    uint64_t k = work->tid;
    auto hybrid_log_persistence_callback = [](Status result, uint64_t persistent_serial_num) {
        if (result != Status::Ok) {
            printf("Thread %" PRIu32 " reports checkpoint failed.\n",
                   Thread::id());
        } else {
            printf("Thread %" PRIu32 " reports persistence until %" PRIu64 "\n",
                   Thread::id(), persistent_serial_num);
        }
    };
    store.StartSession();
    //  while (stopMeasure.load(memory_order_relaxed) == 0) {
    for (uint64_t i = (k) * total_count / thread_number; i < (k + 1) * total_count / thread_number; i++) {
#if TEST_LOOKUP
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<ReadContext> context{ctxt};
        };
#if CONTEXT_TYPE == 0
        ReadContext context{loads[i]};

        Status result = store.Read(context, callback, 1);
        if (result == Status::Ok)
            hit++;
        else
            fail++;
#elif CONTEXT_TYPE == 2
        ReadContext context(loads[i]);

        Status result = store.Read(context, callback, 1);
        if (result == Status::Ok && *(uint64_t *) (context.output_bytes) == total_count - loads[i])
            hit++;
        else
            fail++;
#endif
#else
        auto callback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<UpsertContext> context{ctxt};
        };
#if CONTEXT_TYPE == 0
        //UpsertContext context{loads[i], loads[i]};
        UpsertContext context{i, i};
#elif CONTEXT_TYPE == 2
        UpsertContext context(loads[i], 8);
        context.reset((uint8_t *) (content + i));
#endif

        //Status stat = store.Upsert(context, callback, 1);
        Status stat = store.UpsertT(context, callback, 1,thread_number);
        if (stat == Status::NotFound)
            fail++;
        else
            hit++;
#endif
        /*
        if (i % kCompletePendingInterval == 0) {
            store.CompletePending(false);
        } else if (i % kRefreshInterval == 0) {
            store.Refresh();
        }

        if (i % kCheckpointInterval == 0) {
            Guid token;
            if (store.Checkpoint(nullptr, hybrid_log_persistence_callback, token))
                printf("Thread= %d Calling Checkpoint(), token = %s\n", work->tid, token.ToString().c_str());
        }
        */
    }
    //  }
    store.StopSession();
    //long elipsed;
    elipsed= tracer.getRunTime();
    output[work->tid] << work->tid << " " << elipsed << " " << hit << endl;
    __sync_fetch_and_add(&total_time, elipsed);
    __sync_fetch_and_add(&success, hit);
    __sync_fetch_and_add(&failure, fail);
}

void prepare() {
    cout << "prepare" << endl;
    workers = new pthread_t[thread_number];
    parms = new struct target[thread_number];
    output = new stringstream[thread_number];
    for (uint64_t i = 0; i < thread_number; i++) {
        parms[i].tid = i;
        parms[i].store = &store;

        parms[i].insert = (uint64_t *) calloc(total_count / thread_number, sizeof(uint64_t *));
        char buf[DEFAULT_STR_LENGTH];
        for (int j = 0; j < total_count / thread_number; j++) {
            std::sprintf(buf, "%d", i + j * thread_number);
            parms[i].insert[j] = j;
        }

    }
#if CONTEXT_TYPE == 2
    content = new uint64_t[total_count];
    for (long i = 0; i < total_count; i++) {
        content[i] = total_count - loads[i];
    }
#endif
}

void finish() {
    cout << "finish" << endl;
    for (int i = 0; i < thread_number; i++) {
        delete[] parms[i].insert;
    }
    delete[] parms;
    delete[] workers;
    delete[] output;
#if CONTEXT_TYPE == 2
    delete[] content;
#endif
}

void multiWorkers() {
    output = new stringstream[thread_number];
    Tracer tracer;
    tracer.startTime();
    /*
    for (int i = 0; i < thread_number; i++) {
        pthread_create(&workers[i], nullptr, insertWorker, &parms[i]);
    }
    for (int i = 0; i < thread_number; i++) {
        pthread_join(workers[i], nullptr);
    }
    cout << "Insert " << exists << " " << tracer.getRunTime() << endl;
    */

    Timer timer;
    timer.start();
    for (int i = 0; i < thread_number; i++) {
        pthread_create(&workers[i], nullptr, measureWorker, &parms[i]);
    }
    //while (timer.elapsedSeconds() < timer_range) {
    //    sleep(1);
    //  }
    // stopMeasure.store(1, memory_order_relaxed);
    for (int i = 0; i < thread_number; i++) {
        pthread_join(workers[i], nullptr);
        string outstr = output[i].str();
        cout << outstr;
    }
    cout << "Gathering ..." << endl;
}

int main(int argc, char **argv) {
    if (argc > 4) {
        thread_number = std::atol(argv[1]);
        key_range = std::atol(argv[2]);
        total_count = std::atol(argv[3]);
        rounds = std::atol(argv[4]);

    }
    cout << " threads: " << thread_number << " range: " << key_range << " count: " << total_count << " time: "
         << timer_range << endl;
    loads = (uint64_t *) calloc(total_count, sizeof(uint64_t));
    //for (int i = 0; i < total_count; i++) loads[i] = i;
    UniformGen<uint64_t>::generate(loads, key_range, total_count);
    prepare();
    if (argc>5){
        //string str= reinterpret_cast<const char *>(std::atol(argv[5]));
        Guid token = Guid::Parse(argv[5]);
        RecoverAndTest(token,token);
        goto y;
    }
    // simpleInsert();
    cout << "multiinsert" << endl;
    multiWorkers();
    cout << "operations: " << success << " failure: " << failure << " throughput: "
         << (double) (success + failure) * thread_number / total_time << endl;
    stopMeasure.store(0);
    multiPoints();
    //cout << "simple read" << endl;
    //simpleRead();
    stopMeasure.store(0);
    GC();
    cout << "simple read" << endl;
    simpleRead();
    //cout<<"simple insert"<<endl;
    //simpleInsert1();
    //cout << "simple read" << endl;
    //simpleRead();

    for(int i=0;i<rounds;i++) {

        stopMeasure.store(0);
        multiPoints();
        //RecoverAndTest(retoken, retoken);
        // Populate();
        cout << "after checkpoint multiinsert" << endl;
        success = 0;
        failure = 0;
        total_time = 0;
        multiWorkers();
        cout << "operations: " << success << " failure: " << failure << " throughput: "
             << (double) (success + failure) * thread_number / total_time << endl;

        success = 0;
        failure = 0;
        total_time = 0;
        cout << "multiinsert" << endl;
        multiWorkers();
        cout << "operations: " << success << " failure: " << failure << " throughput: "
             << (double) (success + failure) * thread_number / total_time << endl;

    }

    y:
    free(loads);
    finish();
    return 0;
}
