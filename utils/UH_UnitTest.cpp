//
// Created by Michael on 2019-07-10.
//

#include <functional>
#include <iostream>
#include <stack>
#include <thread>
#include <boost/lockfree/stack.hpp>
#include "tracer.h"
#include "universal_hash_table.h"

using namespace std;
using namespace boost;
using namespace neatlib;

char *dummy[]{
        "abwerewrsdfrwereweewrewrrewrwerw",
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw",
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw",
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw",
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
        "abwerewrsdfrwereweewrewrrewrwerw"
};

void simpleOperationTests() {
    UniversalHashTable<char *, char *, std::hash<char *>, 4, 16> mhash;
    for (int i = 0; i < 5; i++) {
        mhash.Insert(dummy[i], dummy[i]);
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
    for (int i = 0; i < 1000000; i++) {
        s->push(i);
    }
    for (int i = 0; i < 1000000; i++) {
        s->pop();
    }
}

void bstWorker(boost::lockfree::stack<int> *s) {
    for (int i = 0; i < 100000000; i++) {
        s->push(i);
    }
    for (int i = 0; i < 100000000; i++) {
        int v = -1;
        s->pop(v);
    }
}

const int thread_number = 1;

void boostStackVSstdStack(bool usingStd) {
    std::thread *workers[thread_number];
    if (usingStd) {
        std::stack<int> stdStack;
        stdWorker(&stdStack);
    } else {
        boost::lockfree::stack<int> bstStack(128);
        for (int t = 0; t < thread_number; t++) {
            std::thread worker(bstWorker, &bstStack);
            workers[t] = &worker;
        }
        for (int t = 0; t < thread_number; t++) {
            workers[t]->join();
        }
    }
}

int main(int argc, char **argv) {
    Tracer tracer;
    tracer.startTime();
    simpleOperationTests();
    cout << "Ist: " << tracer.getRunTime() << endl;
    tracer.startTime();
    boostStackVSstdStack(true);
    cout << "Std: " << tracer.getRunTime() << endl;
    tracer.startTime();
    boostStackVSstdStack(false);
    cout << "Bst: " << tracer.getRunTime() << endl;
    return 0;
}