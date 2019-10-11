//
// Created by iclab on 10/11/19.
//

#include <atomic>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <thread>
#include "gtest/gtest.h"
#include "MaskRWPtr.h"

TEST(MaskRWPtr, UninRWTest) {
    uint64_t ul = 0;
    std::cout << ul << std::endl;
    std::atomic<uint64_t> aul(ul);
    aul.store(1);
    std::cout << aul << ":" << ul << std::endl;

    MaskRWPtr ptr;
    ptr.rwptr = 0x0;
    std::cout << ptr.rwptr << std::endl;
    std::atomic<MaskRWPtr> control0(ptr);
    MaskRWPtr newptr = ptr;
    newptr.control0++;
    control0.compare_exchange_strong(newptr, ptr);
    std::cout << ptr.rwptr << ":" << ptr.control0 << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}