//
// Created by Michael on 2019-10-15.
//

#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <thread>
#include "gtest/gtest.h"
#include "folly/AtomicHashMap.h"
#include "folly/AtomicUnorderedMap.h"

TEST(FollyTest, UnitOperation) {
    folly::AtomicHashMap<uint64_t, uint64_t> fmap(128);
    fmap.insert(1, 1);
    ASSERT_EQ(fmap.find(1)->second, 1);
    fmap.insert(1, 2);
    ASSERT_EQ(fmap.find(1)->second, 2);
    fmap.erase(1);
    ASSERT_EQ(fmap.find(1)->second, 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}