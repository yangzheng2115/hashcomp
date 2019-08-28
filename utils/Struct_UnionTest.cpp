//
// Created by lwh on 19-8-29.
//

#include <atomic>
#include <iostream>
#include <cstdint>
#include <thread>
#include <deque>

using namespace std;

const short TN = 8;

const short N3 = 24;
const short N2 = 16;
const short N1 = 8;
const short N0 = 0;
const short BL = 8;

union uint32_union {
    struct {
        uint32_t byte3 : BL;
        uint32_t byte2 : BL;
        uint32_t byte1 : BL;
        uint32_t byte0 : BL;
    };
    uint32_t reserve;
};

auto atomicSet = [](atomic<uint32_union> *uu, int tid) {
//void atomicSet(atomic<uint32_union> *uu, int tid) {
    for (int i = 0; i < (1 << 6 - 1); i++) {
        switch (tid % 4) {
            case 0:
                while (true) {
                    uint32_union uuo = uu->load();
                    uint32_union uun = uu->load();
                    uun.reserve -= (1 << N0);
                    if (uu->compare_exchange_weak(uuo, uun))
                        break;
                }
                break;
            case 1:
                while (true) {
                    uint32_union uuo = uu->load();
                    uint32_union uun = uu->load();
                    uun.reserve -= (1 << N1);
                    if (uu->compare_exchange_weak(uuo, uun))
                        break;
                }
                break;
            case 2:
                while (true) {
                    uint32_union uuo = uu->load();
                    uint32_union uun = uu->load();
                    uun.reserve -= (1 << N2);
                    if (uu->compare_exchange_weak(uuo, uun))
                        break;
                }
                break;
            case 3:
                while (true) {
                    uint32_union uuo = uu->load();
                    uint32_union uun = uu->load();
                    uun.reserve -= (1 << N3);
                    if (uu->compare_exchange_weak(uuo, uun))
                        break;
                }
                break;
            default:
                break;
        }
    }
};

int main(int argc, char **argv) {
    uint32_union uu;
    uu.reserve = -1;
    cout << sizeof(uu) << " " << uu.byte0 << " " << uu.byte1 << " " << uu.byte2 << " " << uu.byte3 << " " << uu.reserve
         << endl;
    uu.byte0 = 0x7f;
    cout << sizeof(uu) << " " << uu.byte0 << " " << uu.byte1 << " " << uu.byte2 << " " << uu.byte3 << " " << uu.reserve
         << endl;
    deque<thread> threads;
    atomic<uint32_union> auu(uu);
    for (int i = 0; i < TN; i++) {
        threads.emplace_back(atomicSet, &auu, i);
    }
    for (auto &thread: threads) {
        thread.join();
    }
    cout << sizeof(uu) << " " << uu.byte0 << " " << uu.byte1 << " " << uu.byte2 << " " << uu.byte3 << " " << uu.reserve
         << endl;
    cout << sizeof(auu) << " " << auu.load().byte0 << " " << auu.load().byte1 << " " << auu.load().byte2 << " "
         << auu.load().byte3 << " " << auu.load().reserve << endl;
    return 0;
}