//
// Created by iclab on 10/11/19.
//

#include <atomic>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <thread>
#include <src/mhash/util/MaskRWPtr.h>
#include "gtest/gtest.h"
#include "MaskRWPtr.h"

using namespace std;

TEST(UnionRWTest, MaskByteTest) {
    MaskByte mb;
    mb.byte = 255;
    ASSERT_EQ(mb.byte, 255);
    ASSERT_EQ(mb.bit0, 1);
    mb.bit0 = 0;
    ASSERT_EQ(mb.byte, 254);
    atomic<MaskByte> amb(mb);
    MaskByte revalue;
    revalue.byte = 127;
    amb.store(revalue);
    ASSERT_EQ(amb.load().byte, 127);
    //Testing bit-first/byte-first-initialization
    MaskByte rewind{127};
    amb.store(rewind);
    ASSERT_EQ(amb.load().byte, 1);
    MaskByte reverse{0};
    amb.store(reverse);
    ASSERT_EQ(amb.load().byte, 0);
}

TEST(UnionRWTest, MaskRByteTest) {
    MaskRByte mb;
    mb.byte = 255;
    ASSERT_EQ(mb.byte, 255);
    ASSERT_EQ(mb.bit0, 1);
    mb.bit0 = 0;
    ASSERT_EQ(mb.byte, 254);
    atomic<MaskRByte> amb(mb);
    MaskRByte revalue;
    revalue.byte = 127;
    amb.store(revalue);
    ASSERT_EQ(amb.load().byte, 127);
    //Testing byte-first/bit-first-initialization
    MaskRByte rewind{127};
    amb.store(rewind);
    ASSERT_EQ(amb.load().byte, 127);
    MaskRByte reverse{0};
    amb.store(reverse);
    ASSERT_EQ(amb.load().byte, 0);
}

TEST(UnionRWTest, AtomicBytesTest) {
    ASSERT_EQ(sizeof(AtomicBytes), sizeof(bool) * 8);
    ASSERT_EQ(sizeof(atomic<bool>), sizeof(bool));
    ASSERT_EQ(sizeof(bool), sizeof(uint8_t));
    AtomicBytes amb;
    amb.byte.store(0);
    ASSERT_EQ(amb.bytes.bit4.load(), false); // aka 0
    amb.byte.store(255);
    ASSERT_EQ(amb.bytes.bit0.load(), 255); // Ping!!!
    ASSERT_EQ(amb.bytes.bit4.load(), false); // aka 0
    amb.bytes.bit4.store(false);
    ASSERT_EQ(amb.byte.load(), 255);
}

TEST(UnionRWTest, AtomicMaskByteTest) {
    ASSERT_EQ(sizeof(AtomicMaskUint64), sizeof(uint64_t));
    AtomicMaskUint64 amb;
    amb.dword.store(0xffffffffffffffff);
    ASSERT_EQ(amb.dword.load(), 18446744073709551615LLU);
    ASSERT_EQ(amb.bytes.byte0.load(), 255); // We can use byte#[[0]] as macro
    ASSERT_EQ(amb.bytes.byte7.load(), 255); // We can use byte#[[7]] as macro
    amb.bytes.byte1.fetch_add(1);
    ASSERT_NE(amb.dword, 255);
    ASSERT_NE(amb.dword, 18446744073709551615LLU);
    ASSERT_EQ(amb.dword, -65281);

    amb.dword.store(0xffffffffffffffff);
    amb.dword.fetch_add(1);
    ASSERT_EQ(amb.bytes.byte0, 0);
    ASSERT_EQ(amb.bytes.byte1, 0);
    ASSERT_EQ(amb.bytes.byte2, 0);
    ASSERT_EQ(amb.bytes.byte3, 0);
    ASSERT_EQ(amb.bytes.byte4, 0);
    ASSERT_EQ(amb.bytes.byte5, 0);
    ASSERT_EQ(amb.bytes.byte6, 0);
    ASSERT_EQ(amb.bytes.byte7, 0);
}

TEST(UnionRWTest, AtomicAlignasMaskByteTest) {
    ASSERT_EQ(sizeof(AtomicAlignasMaskUint64), 128 * sizeof(uint64_t));
    AtomicAlignasMaskUint64 amb;
    amb.dword.store(0xffffffffffffffff);
    ASSERT_EQ(amb.dword.load(), 18446744073709551615LLU);
    ASSERT_EQ(amb.bytes.byte0.load(), 255); // We can use byte#[[0]] as macro
    ASSERT_NE(amb.bytes.byte1.load(), 255); // We can use byte#[[1]] as macro
    ASSERT_NE(amb.bytes.byte7.load(), 255); // We can use byte#[[7]] as macro
    amb.bytes.byte1.fetch_add(1);
    ASSERT_NE(amb.dword, 255);
    ASSERT_EQ(amb.dword, 18446744073709551615LLU);
    ASSERT_NE(amb.dword, -65281);

    amb.dword.store(0xffffffffffffffff);
    amb.dword.fetch_add(1);
    ASSERT_EQ(amb.bytes.byte0, 0);
    ASSERT_NE(amb.bytes.byte1, 0);
    ASSERT_NE(amb.bytes.byte2, 0);
    ASSERT_EQ(amb.bytes.byte3, 0);
    ASSERT_NE(amb.bytes.byte4, 0);
    ASSERT_NE(amb.bytes.byte5, 0);
    ASSERT_NE(amb.bytes.byte6, 0);
    ASSERT_NE(amb.bytes.byte7, 0);
}

TEST(UnionRWTest, MaskRWPtrTest) {
    uint64_t ul = 0;
    ASSERT_EQ(ul, 0);
    std::atomic<uint64_t> aul(ul);
    aul.store(1);
    ASSERT_NE(ul, aul);

    MaskRWPtr ptr;
    ptr.rwptr = 0x0;
    ASSERT_EQ(ptr.rwptr, 0LLU);
    std::atomic<MaskRWPtr> control0(ptr);
    MaskRWPtr newptr = ptr;
    newptr.control0++;
    ASSERT_EQ(newptr.control0, 1LLU);
    control0.compare_exchange_strong(newptr, ptr);
    ASSERT_EQ(ptr.rwptr, 0LLU);
    ASSERT_EQ(ptr.control0, 0LLU);
}

TEST(UnionRWTest, MaskUint64Test) {
    MaskUint64 mu;
    mu.dword = 0;
    ASSERT_EQ(mu.bytes.byte0, 0);
    ASSERT_EQ(mu.bytes.byte1, 0);
    ASSERT_EQ(mu.bytes.byte2, 0);
    ASSERT_EQ(mu.bytes.byte3, 0);
    ASSERT_EQ(mu.bytes.byte4, 0);
    ASSERT_EQ(mu.bytes.byte5, 0);
    ASSERT_EQ(mu.bytes.byte6, 0);
    ASSERT_EQ(mu.bytes.byte7, 0);
    ASSERT_EQ(mu.dword, 0);
    atomic_add(&mu.dword, 1);
    ASSERT_EQ(mu.bytes.byte0, 1);
    ASSERT_EQ(mu.bytes.byte1, 0);
    ASSERT_EQ(mu.dword, 1);
    atomic_mask_add(&mu, 0, 1);
    ASSERT_EQ(mu.bytes.byte0, 2);
    ASSERT_EQ(mu.bytes.byte1, 0);
    atomic_mask_add(&mu, 1, 1);
    ASSERT_EQ(mu.bytes.byte0, 2);
    ASSERT_EQ(mu.bytes.byte1, 1);
    ASSERT_EQ(mu.dword, 258);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}