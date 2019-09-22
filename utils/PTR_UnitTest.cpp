//
// Created by Michael on 2019-09-21.
//

#include <chrono>
#include <iostream>
#include <sstream>
#include <memory>
#include <thread>
#include <vector>
#import "tracer.h"

using namespace std;

//constexpr chrono::nanoseconds sleepNano = chrono::nanoseconds(1000);
constexpr chrono::duration<long long int> sleepSeconds = chrono::seconds(1);

int pdegree = 4;

constexpr int threadOprs = (1 << 2);

constexpr long iteration = (1 << 30);

struct node {
    uint64_t element = 0;
public:
    node(uint64_t e) : element(e) {}

    /*node &operator=(node &n) {
        element = n.element;
        return *this;
    }

    node &operator=(const node &n) {
        element = n.element;
        return *this;
    }

    node *operator=(node *n) {
        element = n->element;
        return this;
    }

    node *operator=(const node *n) {
        element = n->element;
        return this;
    }*/

    node &operator*() {
        return *this;
    }

    node(node &n) : element(n.element) {}
};

struct node wallclock(0);

//unique_ptr<node> pwc = unique_ptr<node>(new node(0), std::default_delete<node>());
volatile atomic<node *> pwc(&wallclock);
stringstream *output;

void uniquePtrWorker(int tid) {
    Tracer tracer;
    tracer.startTime();
    /*//unique_ptr<node> pwc = make_unique<node>(0);
    //node *pcw = static_cast<node *>(pwc.get());
    //unique_ptr<node &> pcw = pwc;

    ////auto pcw = static_cast<node *>(pwc.get());
    //*/
    for (long r = 0; r < (iteration / pdegree); r++) {
        register node *dummy = nullptr;
        register node *pcw = nullptr;
        do {
            pcw = pwc;
        } while (pcw == nullptr || !pwc.compare_exchange_weak(pcw, dummy, memory_order_release));

        //iter:
        //unique_ptr<node> pcw = move(pwc);
        //auto ncw = static_cast<node *>(pcw.get());
        //if (pcw == nullptr) goto iter;
        for (int i = 0; i < threadOprs; i++) {
            /*//pwc.operator->()->element++;
            //ncw->element++;
            ////pcw->element++;
            //wallclock.element++;*/

            pcw->element++;
            /*if (ncw != nullptr) {
                ncw->element++;
            } else {
                //this_thread::sleep_for(sleepSeconds);
                goto iter;
            }*/
            //if (i % 1000000) cout << tid << ":" << i << endl;

            /*//pcw->element++;
             //pwc.operator*().element++;
             //pcw.operator*().element++;
             //pcw.operator*()++;
             //*pcw++;
             //pwc.operator*()++;*/
            //cout << tid << ":" /**pwc*/ << ncw->element << endl;// ":" << pwc << endl;
            //this_thread::sleep_for(sleepSeconds);
        }
        pwc.store(pcw);
        /*pwc = move(pcw);*/
    }
    output[tid] << tid << ":" << tracer.getRunTime() << endl;
}

void uniquePtrTests() {
    std::vector<thread> workers;
    Tracer tracer;
    cout << "Intend: " << iteration * threadOprs << " " << pdegree << endl;
    tracer.startTime();
    for (int t = 0; t < pdegree; t++) {
        workers.push_back(std::thread(uniquePtrWorker, t));
    }
    for (int t = 0; t < pdegree; t++) {
        workers[t].join();
        cout << "\t" << output[t].str();
    }

    /*auto n = static_cast<node *>(pwc.get());
    while (n == nullptr) {
        n = static_cast<node *>(pwc.get());
    }*/

    cout << "Multi-thread pdegree: " << pdegree << "<->" << wallclock.element << "<>" /*<< n->element*/ << ":"
         << tracer.getRunTime() << endl;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        pdegree = std::atoi(argv[1]);
    }
    output = new stringstream[pdegree];
    uniquePtrTests();
    delete[] output;
    return 0;
}