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

const size_t total_item = (1 << 26);

const int thread_number = 8;

void stdWorker(std::stack<int> *s) {
    for (int i = 0; i < total_item; i++) {
        s->push(i);
    }
    for (int i = 0; i < total_item; i++) {
        s->pop();
    }
}

void bstWorker(boost::lockfree::stack<int> *s) {
    for (int i = 0; i < total_item / thread_number; i++) {
        s->push(i);
    }
    for (int i = 0; i < total_item / thread_number; i++) {
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
        for (int t = 0; t < thread_number; t++) {
            workers.push_back(std::thread(bstWorker, &bstStack));
        }
        for (int t = 0; t < thread_number; t++) {
            workers[t].join();
        }
    }
}

int main(int argc, char **argv) {
    Tracer tracer;
    tracer.startTime();
    simpleOperationTests();
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