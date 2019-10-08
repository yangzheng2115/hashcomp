//
// Created by iclab on 10/7/19.
//

#include <atomic>
#include <iostream>
#include <thread>
#include "faster.h"
#include "gtest/gtest.h"
#include "cvkvcontext.h"

using namespace std;
using namespace FASTER::api;
using namespace FASTER::io;
using namespace FASTER::core;

atomic<uint64_t> tick;

TEST(ContextVerify, ConcurrentRU) {
    typedef QueueIoHandler handler_t;
    typedef FileSystemDisk<handler_t, 1073741824ull> disk_t;
    constexpr uint64_t rounds = (1 << 20);
    constexpr uint64_t bfsize = 256;
    using store_t = FasterKv<Key, Value, disk_t>;
    size_t init_size = next_power_of_two(512 / 2);
    store_t store{init_size, 17179869184, "storage"};
    Value loads[bfsize];
    uint64_t values[bfsize];
    for (uint64_t i = 0; i < bfsize; i++) {
        values[i] = bfsize - i % bfsize;
        loads[i].reset((uint8_t *) (values + i), 8);
    }

    auto updater = [](Value *loads, store_t *store, atomic<uint64_t> *tick) {
        for (int i = 0; i < rounds; i++) {
            auto callback = [](IAsyncContext *ctxt, Status result) {
                CallbackContext<UpsertContext> context{ctxt};
            };
            UpsertContext context(0, 8);
            context.reset(loads[i % bfsize].get());
            uint64_t *value = (uint64_t *) (loads[i % bfsize].get());
            ASSERT_EQ(*value, 256 - i % 256);
            Status stat = store->Upsert(context, callback, 1);
            tick->fetch_add(1);
        }
    };

    auto reader = [](store_t *store, atomic<uint64_t> *tick) {
        uint64_t oldtick = 0;
        for (int i = 0; i < (1 << 6); i++) {
            auto callback = [](IAsyncContext *ctxt, Status result) {
                CallbackContext<ReadContext> context{ctxt};
            };
            ReadContext rc(0);
            while (tick->load() - oldtick < (1 << 14) && tick->load() < rounds) {
                this_thread::yield();
            }
            oldtick = tick->load();
            Status stat = store->Read(rc, callback, 1);

            // Nice print
            if (i % 8 == 0 && i != 0) cout << endl;
            cout.width(7);
            cout << tick->load();
            cout;
            cout << ":";
            cout.width(2);
            cout << i;
            cout;
            cout << "<->";
            cout.width(3);
            cout << *((uint64_t *) rc.output_bytes);
            cout;
            cout << " ";
        }
        cout << endl;
    };

    std::thread updateThread(updater, loads, &store, &tick);
    std::thread readThread(reader, &store, &tick);
    updateThread.join();
    readThread.join();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}