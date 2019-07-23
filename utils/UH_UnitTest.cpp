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

uint64_t total = 100000000;
int pdegree = 4;
int simple = 0;
long total_runtime = 0;
long max_runtime = 0;
long min_runtime = std::numeric_limits<long>::max();

void simpleOperationTests() {
    char *left = "abcde";
    char *middle = "abcd";
    char *right = "abcde";
    std::hash<char *> hasher;
    std::equal_to<char *> et;
    cout << hasher(left) << " " << hasher(right) << " " << hasher(middle) << endl;
    cout << et(left, right) << " " << et(left, middle) << " " << true << endl;
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
    Tracer tracer;
    tracer.startTime();
    if (simple) {
        simpleOperationTests();
    } else {
        insert();
    }
    cout << "Ist: " << tracer.getRunTime() << endl;
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