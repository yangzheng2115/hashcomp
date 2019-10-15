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
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}