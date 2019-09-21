//
// Created by Michael on 2019-09-21.
//

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

using namespace std;

//constexpr chrono::nanoseconds sleep = chrono::nanoseconds(1000);
constexpr chrono::duration<long long int> sleep = chrono::seconds(1);

constexpr int pdegree = 4;

constexpr int round = 3;

struct node {
    int element = 0;
public:
    node(int e) : element(e) {}

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

unique_ptr<node> pwc = unique_ptr<node>(new node(0), std::default_delete<node>());

void uniquePtrWorker(int tid) {
    /*//unique_ptr<node> pwc = make_unique<node>(0);
    //node *pcw = static_cast<node *>(pwc.get());
    //unique_ptr<node &> pcw = pwc;

    ////auto pcw = static_cast<node *>(pwc.get());
    //*/
    iter:
    unique_ptr<node> pcw = move(pwc);
    auto ncw = static_cast<node *>(pcw.get());
    for (int i = 0; i < round; i++) {
        /*//pwc.operator->()->element++;
        //ncw->element++;
        ////pcw->element++;
        //wallclock.element++;*/

        if (ncw != nullptr) {
            ncw->element++;
        } else {
            this_thread::sleep_for(sleep);
            goto iter;
        }

        /*//pcw->element++;
         //pwc.operator*().element++;
         //pcw.operator*().element++;
         //pcw.operator*()++;
         //*pcw++;
         //pwc.operator*()++;*/
        cout << tid << ":" /**pwc*/ << ncw->element << endl;// ":" << pwc << endl;
        this_thread::sleep_for(sleep);
    }
    pwc = move(pcw);
}

void uniquePtrTests() {
    std::vector<thread> workers;
    for (int t = 0; t < pdegree; t++) {
        workers.push_back(std::thread(uniquePtrWorker, t));
    }
    for (int t = 0; t < pdegree; t++) {
        workers[t].join();
    }

    auto n = static_cast<node *>(pwc.get());
    while (n == nullptr) {
        n = static_cast<node *>(pwc.get());
    }

    cout << "Multi-thread pdegree: " << pdegree << "<->" << wallclock.element << "<>" << n->element << endl;
}

int main(int argc, char **argv) {
    uniquePtrTests();
    return 0;
}