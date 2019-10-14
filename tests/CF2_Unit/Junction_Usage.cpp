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
#include "junction/ConcurrentMap_Grampa.h"

TEST(JunctionTests, LeapfrogOperations) {
    junction::ConcurrentMap_Leapfrog<uint64_t, uint64_t> jmap(128);
    jmap.assign(1, 1);
    auto v = jmap.get(1);
    ASSERT_EQ(v, 1);
    jmap.exchange(1, 2);
    jmap.find(1);
    ASSERT_EQ(jmap.get(1), 2);
}

TEST(JunctionTests, GrampaOperations) {
    junction::QSBR::Context context = junction::DefaultQSBR.createContext();
    junction::ConcurrentMap_Grampa<int, int> jmap(128);
    for (int i = 0; i < 128; i++) jmap.assign(i, i);
    jmap.find(1);
    ASSERT_EQ(jmap.get(1), 2);
    jmap.exchange(1, 2);
    jmap.find(1);
    ASSERT_EQ(jmap.get(1), 2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}