//
// Created by lwh on 19-7-15.
//

#include "index_hash.h"

uint64_t *loads = nullptr;

void initLoads() {
    g_thread_cnt = 1;
    g_part_cnt = 1;
    warmup_finish = false;
    loads = new uint64_t[1000000];

    for (int i = 0; i < 1000000; i++) {
        loads[i] = i;
    }
}

void simpleOperationTests() {
    IndexHash xh;
    //xh.init(10000000LLU, g_part_cnt);
    xh.init(g_part_cnt, nullptr, 10000000LLU);

    for (int i = 0; i < 1000000; i++) {
        itemid_t *m_item = (itemid_t *) mem_allocator.alloc(sizeof(itemid_t), 0);
        m_item->init();
        m_item->type = DT_row;
        m_item->location = &loads[i];
        m_item->valid = true;
        xh.index_insert(loads[i], m_item);
    }
}

int main(int argc, char **argv) {
    initLoads();
    simpleOperationTests();
    return 0;
}