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

struct Foo {
    uint64_t value;
public:
    Foo(uint64_t v) : value(v) {}

    uint64_t get() { return value; }
};

TEST(JunctionTests, LeapfrogOperations) {
    junction::QSBR::Context context = junction::DefaultQSBR.createContext();
    junction::ConcurrentMap_Leapfrog<uint64_t, Foo *> jmap(128);
    jmap.assign(1, new Foo(1));
    auto v = jmap.get(1);
    ASSERT_EQ(v->get(), 1);
    jmap.exchange(1, new Foo(2));
    jmap.find(1);
    ASSERT_EQ(jmap.get(1)->get(), 2);
    junction::DefaultQSBR.update(context);
    junction::DefaultQSBR.destroyContext(context);
}

TEST(JunctionTests, GrampaOperations) {
    junction::QSBR::Context context = junction::DefaultQSBR.createContext();
    junction::ConcurrentMap_Grampa<int, Foo *> jmap(128);
    for (int i = 0; i < 128; i++) jmap.assign(i, new Foo(i));
    jmap.find(1);
    ASSERT_EQ(jmap.get(1)->get(), 1);
    jmap.exchange(1, new Foo(2));
    jmap.find(1);
    ASSERT_EQ(jmap.get(1)->get(), 2);
    junction::DefaultQSBR.update(context);
    junction::DefaultQSBR.destroyContext(context);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}