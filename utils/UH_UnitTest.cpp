//
// Created by Michael on 2019-07-10.
//

#include <functional>
#include <iostream>
#include <stack>
#include <thread>
#include <boost/lockfree/stack.hpp>
#include "tracer.h"
#include "basic_hash_table.h"
#include "concurrent_hash_table.h"
#include "universal_hash_table.h"
#include "ycsbHelper.h"

#if defined(__linux__)

#include <cpuid.h>

#endif

using namespace std;
using namespace boost;
using namespace neatlib;

#define VARIANT_FIELD 1
#define CACHE_RESERVE (1 << 8)

uint64_t total = 10000000;
int pdegree = 4;
int simple = 0;
long total_runtime = 0;
long max_runtime = 0;
long min_runtime = std::numeric_limits<long>::max();

void simpleOperationTests() {
    const char *cleft = "Meet the new boss...";
    const char *cmiddle = "Meet the new boss";
    const char *cright = "Meet the new boss...";
    std::hash<const char *> chasher;
    std::equal_to<const char *> cet;
    cout << chasher(cleft) << " " << chasher(cright) << " " << chasher(cmiddle) << endl;
    cout << cet(cleft, cright) << " " << cet(cleft, cmiddle) << " " << true << endl;

    string sleft("Meet the new boss...");
    string smiddle("Meet the new boss");
    string sright("Meet the new boss...");
    const char *ccleft = sleft.c_str();
    const char *ccmiddle = smiddle.c_str();
    const char *ccright = sright.c_str();
    std::hash<const char *> cchasher;
    std::equal_to<const char *> ccet;
    cout << chasher(ccleft) << " " << chasher(ccright) << " " << chasher(ccmiddle) << endl;
    cout << cchasher(ccleft) << " " << cchasher(ccright) << " " << cchasher(ccmiddle) << endl;
    cout << ccet(ccleft, ccright) << " " << ccet(ccleft, ccmiddle) << " " << true << endl;
    cout << strcmp(cleft, ccleft) << " " << std::strcmp(cleft, ccleft) << " " << std::strcmp(cleft, cmiddle) << " "
         << std::strcmp(cmiddle, cleft) << " " << (int) '.' << endl;

    string left = "Meet the new boss...";
    string middle = "Meet the new boss";
    string right = "Meet the new boss...";
    std::hash<string> hasher;
    std::equal_to<string> et;
    cout << hasher(left.c_str()) << " " << hasher(right.c_str()) << " " << hasher(middle.c_str()) << endl;
    cout << hasher(left) << " " << hasher(right) << " " << hasher(middle) << endl;
    cout << et(left, right) << " " << et(left, middle) << " " << true << endl;

    // Notice here that uint64_t cannot generate random hash key.
    uint64_t ileft = 123456789LLU;
    uint64_t imiddle = 1234567890LLU;
    uint64_t iright = 123456789LLU;
    std::hash<uint64_t> ihasher;
    std::equal_to<uint64_t> iet;
    cout << ihasher(ileft) << " " << ihasher(iright) << " " << ihasher(imiddle) << endl;
    cout << iet(ileft, iright) << " " << iet(ileft, imiddle) << " " << true << endl;

    UniversalHashTable<char *, char *, std::hash<char *>, 4, 16> uhash;
    for (int i = 0; i < 5; i++) {
        uhash.Insert(dummy[i], dummy[i]);
    }
    ConcurrentHashTable<char *, char *, std::hash<char *>, 4, 16> chash;
    for (int i = 0; i < 5; i++) {
        chash.Insert(dummy[i], dummy[i]);
    }
    BasicHashTable<char *, char *, std::hash<char *>, std::equal_to<char *>,
            std::allocator<std::pair<const char *, char *>>, 4> bhash;
    for (int i = 0; i < 5; i++) {
        bhash.Insert(dummy[i], dummy[i]);
    }
}

/*template<typename S>
void stackWorker(S &s) {
    for (int i = 0; i < 1000000; i++) {
        s.push(i);
    }
    for (int i = 0; i < 1000000; i++) {
        //s.pop();
    }
}*/

void stdWorker(std::stack<int> *s) {
    for (int i = 0; i < total; i++) {
        s->push(i);
    }
    for (int i = 0; i < total; i++) {
        s->pop();
    }
}

void bstWorker(boost::lockfree::stack<int> *s) {
    for (int i = 0; i < total / pdegree; i++) {
        s->push(i);
    }
    for (int i = 0; i < total / pdegree; i++) {
        int v = -1;
        s->pop(v);
    }
}

void boostStackVSstdStack(bool usingStd) {
    std::vector<std::thread> workers;
    if (usingStd) {
        std::stack<int> stdStack;
        stdWorker(&stdStack);
    } else {
        boost::lockfree::stack<int> bstStack(128);
        for (int t = 0; t < pdegree; t++) {
            workers.push_back(std::thread(bstWorker, &bstStack));
        }
        for (int t = 0; t < pdegree; t++) {
            workers[t].join();
        }
    }
}

void newWorker(bool inBatch, int tid) {
    typedef UniversalHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16>::data_node datanode;
    datanode **loads = new datanode *[total];
    if (inBatch) {
        datanode *cache;
        size_t cursor = CACHE_RESERVE;
        std::hash<uint64_t> hasher;
        for (int i = 0; i < total; i++) {
            if (cursor == CACHE_RESERVE) {
                cache = static_cast<datanode *>(malloc(sizeof(datanode) * CACHE_RESERVE));
                cursor = 0;
            }
            loads[i] = new(cache + cursor)datanode(i, i);
            if (i == (pdegree - 1) && (pdegree - 1) == tid)
                cout << loads[i]->get().first << " " << loads[i]->hash() << " " << hasher(i) << endl;
        }
        for (int i = 0; i < total; i++) {
            // Not very sure whether loads[i] can be deleted here.
            loads[i]->~datanode();
        }
        for (int i = cursor; i < CACHE_RESERVE; i++) {
            cache[i].~datanode();
        }
    } else {
        for (int i = 0; i < total; i++) {
            loads[i] = new datanode(i, i, sizeof(uint64_t));
        }
        for (int i = 0; i < total; i++) {
            delete loads[i];
        }
    }
    delete[] loads;
}

void concurrentDataAllocate(bool inBatch) {
    std::vector<std::thread> workers;
    for (int t = 0; t < pdegree; t++) {
        workers.push_back(std::thread(newWorker, inBatch, t));
    }
    for (int t = 0; t < pdegree; t++) {
        workers[t].join();
    }
}

struct paramstruct {
    int tid;
#if VARIANT_FIELD
    UniversalHashTable<const char *, const char *, std::hash<char *>, 4, 16> *xh;
#else
    UniversalHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16> *xh;
#endif
    long runtime = 0;
};

#if VARIANT_FIELD
UniversalHashTable<const char *, const char *, std::hash<char *>, 4, 16> *xh;
#else
UniversalHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16> *xh;
#endif

void *worker(void *args) {
    paramstruct *param = static_cast<paramstruct *>(args);
    Tracer tracer;
    tracer.startTime();

    for (int i = param->tid; i < total; i += pdegree) {
#if VARIANT_FIELD
        const char *key = std::to_string(i).c_str();
        param->xh->Insert(key, key);
#else
        param->xh->Insert(i, i);
#endif
    }

    param->runtime = tracer.getRunTime();
}

void insert() {
    total_runtime = 0;
    pthread_t threads[pdegree];
    paramstruct params[pdegree];
#if VARIANT_FIELD
    xh = new UniversalHashTable<const char *, const char *, std::hash<char *>, 4, 16>();
#else
    xh = new UniversalHashTable<uint64_t, uint64_t, std::hash<uint64_t>, 4, 16>();
#endif

    for (int i = 0; i < pdegree; i++) {
        params[i].tid = i;
        params[i].xh = xh;
        pthread_create(&threads[i], nullptr, worker, &params[i]);
    }

    for (int i = 0; i < pdegree; i++) {
        pthread_join(threads[i], nullptr);
        total_runtime += params[i].runtime;
        if (params[i].runtime > max_runtime) {
            max_runtime = params[i].runtime;
        }
        if (params[i].runtime < min_runtime) {
            min_runtime = params[i].runtime;
        }
    }
}

int main(int argc, char **argv) {
    if (argc > 3) {
        total = std::atol(argv[1]);
        pdegree = std::atoi(argv[2]);
        simple = std::atoi(argv[3]);
    }
    cout << total << " " << pdegree << " " << simple << endl;
    Tracer tracer;
    tracer.startTime();
    if (simple) {
        simpleOperationTests();
    } else {
        insert();
    }
    cout << "Ist: " << tracer.getRunTime() << endl;
    tracer.startTime();
    concurrentDataAllocate(false);
    cout << "new: " << tracer.getRunTime() << endl;
    tracer.startTime();
    concurrentDataAllocate(true);
    cout << "bew: " << tracer.getRunTime() << endl;
#if defined(__linux__)
    cout << "CPUs: " << sysconf(_SC_NPROCESSORS_ONLN) << endl;
    if (sysconf(_SC_NPROCESSORS_ONLN) > 8) {
        tracer.startTime();
        boostStackVSstdStack(true);
        cout << "Std: " << tracer.getRunTime() << endl;
        tracer.startTime();
        boostStackVSstdStack(false);
        cout << "Bst: " << tracer.getRunTime() << endl;
    }
#endif
    return 0;
}