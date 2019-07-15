//
// Created by lwh on 19-7-15.
//

#include "global.h"

class mem_alloc mem_allocator;

bool g_part_alloc = PART_ALLOC;

UInt32 g_thread_cnt = THREAD_CNT;
UInt32 g_part_cnt = PART_CNT;

UInt32 g_init_parallelism = INIT_PARALLELISM;

bool volatile warmup_finish = false;
bool volatile enable_thread_mem_pool = false;