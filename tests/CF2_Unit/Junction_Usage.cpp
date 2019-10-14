//
// Created by Michael on 2019-10-14.
//

#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <thread>
#include "gtest/gtest.h"
#include "junction/QSBR.h"
#include "junction/ConcurrentMap_Leapfrog.h"

TEST(JunctionTests, UnitOperations) {
    junction::ConcurrentMap_Leapfrog<uint64_t, uint64_t> jmap(128);
    jmap.assign(1, 1);
    ASSERT_EQ(jmap.get(1), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}