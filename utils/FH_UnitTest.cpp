//
// Created by lwh on 19-8-30.
//
#include <iostream>
#include "tracer.h"
#include <stdio.h>
#include "faster.h"
#include "kvcontext.h"

using namespace std;
using namespace FASTER::api;

#define DEFAULT_STORE_BASE (1LLU << 1)

#define DEFAULT_STORE_SIZE ((1LLU << 13) + 656) // lower bound of non-conflict capacity

#ifdef _WIN32
typedef hreadPoolIoHandler handler_t;
#else
typedef QueueIoHandler handler_t;
#endif

typedef FileSystemDisk<handler_t, 1073741824ull> disk_t;

using store_t = FasterKv<Key, Value, disk_t>;

size_t init_size = next_power_of_two(DEFAULT_STORE_BASE / 2);

store_t store{init_size, 17179869184, "storage"};

int main(int argc, char **argv) {
    cout << "\t* " << store.Size() << endl;
    cout << "Insert" << endl;
    {
        auto upsertCallback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<UpsertContext> context{ctxt};
        };
        for (uint64_t i = 0; i < DEFAULT_STORE_SIZE; i++) {
            UpsertContext upsertContext{i, i};
            Status uStat = store.Upsert(upsertContext, upsertCallback, 1);
            //cout << "\t" << Utility::retStatus(uStat) << " " << store.Size() << endl;
        }
        UpsertContext upsertContext{0, 0};
        Status uStat = store.Upsert(upsertContext, upsertCallback, 1);
        cout << "\t" << Utility::retStatus(uStat) << " " << store.Size() << " " << store.entrySize() << endl;
    }
    cout << "Read" << endl;
    {
        auto readCallback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<ReadContext> context(ctxt);
        };
        ReadContext readContext(1);
        Status rStat = store.Read(readContext, readCallback, 1);
        assert(rStat == Status::Ok);
        cout << "\t" << Utility::retStatus(rStat) << " " << store.Size() << " " << store.entrySize() << endl;
    }
    cout << "Delete" << endl;
    {
        auto deleteCallback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<DeleteContext> context(ctxt);
        };
        DeleteContext deleteContext(1);
        Status dStat = store.Delete(deleteContext, deleteCallback, 1);
        assert(dStat == Status::Ok);
        cout << "\t" << Utility::retStatus(dStat) << " " << store.Size() << " " << store.entrySize() << endl;
    }
    cout << "Reread" << endl;
    {
        auto readCallback = [](IAsyncContext *ctxt, Status result) {
            CallbackContext<ReadContext> context(ctxt);
        };
        ReadContext readContext(1);
        Status rStat = store.Read(readContext, readCallback, 1);
        assert(rStat == Status::NotFound);
        cout << "\t" << Utility::retStatus(rStat) << " " << store.Size() << " " << store.entrySize() << endl;
    }
    return 0;
}