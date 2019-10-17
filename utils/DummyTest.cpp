//
// Created by iclab on 10/7/19.
//

#include <atomic>
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
    atomic<long long> tick(0);
    cout << tick.load() << endl;
    tick.fetch_add(1);
    cout << tick.load() << endl;
}