//
// Created by Michael on 2019-09-21.
//

#include <chrono>
#include <iostream>
#include <sstream>
#include <memory>
#include <thread>
#include <vector>
#include "tracer.h"

using namespace std;

//constexpr chrono::nanoseconds sleepNano = chrono::nanoseconds(1000);
constexpr chrono::duration<long long int> sleepSeconds = chrono::seconds(1);

int pdegree = 4;

constexpr int threadOprs = (1 << 2);

constexpr long iteration = (1 << 30);

struct alignas(128)node {
    uint64_t element = 0;
public:
    node() : element(0) {}

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
atomic<node *> pwc(&wallclock);
stringstream *output;

void uniquePtrWorker(int tid) {
    Tracer tracer;
    tracer.startTime();
    /*//unique_ptr<node> pwc = make_unique<node>(0);
    //node *pcw = static_cast<node *>(pwc.get());
    //unique_ptr<node &> pcw = pwc;

    ////auto pcw = static_cast<node *>(pwc.get());
    //*/

    int yc = 1;
    for (long r = 0; r < (iteration / pdegree); r++) {
        node *dummy = nullptr;
        /*register node *pcw = nullptr;
        do {
            pcw = pwc;
        } while (pcw == nullptr || !pwc.compare_exchange_weak(pcw, dummy, memory_order_release));*/

        node *pcw = &wallclock;
        while (!pwc.compare_exchange_strong(pcw, dummy)) {
            for (int c = 0; c < yc; c++) this_thread::yield();
            yc *= 2;
            pcw = &wallclock;
        }
        if (yc >= 2) yc /= 2;

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

struct node *wallclocks;

template<class _Tp>
struct alignas(128) alignasatomic : public atomic<_Tp> {
public:
    alignasatomic() {};

    //alignasatomic(_Tp e) : atomic<_Tp>(e) {}

    bool compare_exchange_strong(_Tp &__e, _Tp __d,
                                 memory_order __m = memory_order_seq_cst) _NOEXCEPT {
        return compare_exchange_strong(__e, __d, __m);
    }

    alignasatomic &operator=(alignasatomic &n) {
        this->store(n);
        return *this;
    }

    alignasatomic &operator=(const alignasatomic &n) {
        this->store(n);
        return *this;
    }

    alignasatomic &operator=(_Tp &n) {
        this->store(n);
        return *this;
    }

    alignasatomic &operator=(const _Tp &n) {
        this->store(n);
        return *this;
    }

    alignasatomic *operator=(alignasatomic *n) {
        this->store(*n);
        return this;
    }

    alignasatomic *operator=(const alignasatomic *n) {
        this->store(*n);
        return this;
    }
};

alignas(128)atomic<node *> *pwcs;

void uniquePtrPWorker(int tid) {
    Tracer tracer;
    tracer.startTime();
    atomic<node *> pc(&wallclocks[tid]);

    int yc = 1;
    for (long r = 0; r < (iteration / pdegree); r++) {
        node *dummy = nullptr;

        node *pcw = wallclocks + tid;
        while (!/*pwcs[tid]*/pc.compare_exchange_strong(pcw, dummy)) {
            for (int c = 0; c < yc; c++) this_thread::yield();
            yc *= 2;
            pcw = wallclocks + tid;
        }
        if (yc >= 2) yc /= 2;

        for (int i = 0; i < threadOprs; i++) {
            pcw->element++;
        }
        /*pwcs[tid]*/pc.store(pcw);
        //if (r % 1000000 == 0) cout << tid << ":" << r << endl;
    }
    output[tid] << tid << ":" << tracer.getRunTime() << endl;
}

void uniquePtrPTests() {
    wallclocks = new node[pdegree];
    pwcs = new atomic<node *>[pdegree]alignas(128);
    for (int t = 0; t < pdegree; t++) {
        pwcs[t] = wallclocks + t;
    }
    delete[] output;
    output = new stringstream[pdegree];
    std::vector<thread> workers;
    Tracer tracer;
    cout << "Intend: " << iteration * threadOprs << " " << pdegree << endl;
    tracer.startTime();
    for (int t = 0; t < pdegree; t++) {
        workers.push_back(std::thread(uniquePtrPWorker, t));
    }
    long ec = 0;
    for (int t = 0; t < pdegree; t++) {
        workers[t].join();
        ec += wallclocks[t].element;
        cout << "\t" << output[t].str();
    }

    cout << "Multi-thread: " << pdegree << "<->" << ec << "<>" << ":" << tracer.getRunTime() << endl;
    delete[] wallclocks;
    delete[] pwcs;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        pdegree = std::atoi(argv[1]);
    }
    output = new stringstream[pdegree];
    uniquePtrTests();
    uniquePtrPTests();
    delete[] output;
    return 0;
}