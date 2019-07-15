//
// Created by lwh on 19-7-15.
//

#ifndef HASHCOMP_GLOBAL_H
#define HASHCOMP_GLOBAL_H

#include <assert.h>
#include <stdint.h>
#include <mm_malloc.h>
#include "mem_alloc.h"

#define NOGRAPHITE                  1
#define CL_SIZE                     64
#define CPU_FREQ                    2    // in GHz/s
#define THREAD_CNT                  4
#define INIT_PARALLELISM            40
#define PART_CNT                    1
#define PAGE_SIZE                   4096
#define MEM_ALLIGN                  8
#define PART_ALLOC                  false
#define THREAD_ALLOC                false
#define NO_FREE                     false

#define DEFAULT_TABLE_NAME          "dummy"

using namespace std;

//class mem_alloc;

typedef uint64_t idx_key_t; // key id for index
typedef uint32_t UInt32;
typedef uint64_t UInt64;

extern class mem_alloc mem_allocator;

extern bool g_part_alloc;
extern UInt32 g_thread_cnt;
extern UInt32 g_part_cnt;
extern UInt32 g_thread_cnt;
extern UInt32 g_init_parallelism;

extern bool volatile warmup_finish;
extern bool volatile enable_thread_mem_pool;

enum RC {
    RCOK, Commit, Abort, WAIT, ERROR, FINISH
};

#endif //HASHCOMP_GLOBAL_H
