//
// Created by iclab on 10/10/19.
//

#ifndef HASHCOMP_MASKRWPTR_H
#define HASHCOMP_MASKRWPTR_H

#include <atomic>

constexpr uint8_t BitWidth = 1;

constexpr uint8_t ByteWidth = 8;

enum MaskByteBitType {
    ByteBit0 = 0,
    ByteBit1 = 1,
    ByteBit2 = 2,
    ByteBit3 = 3,
    ByteBit4 = 4,
    ByteBit5 = 5,
    ByteBit6 = 6,
    ByteBit7 = 7
};

union MaskByte {
    struct {
        uint8_t bit0 : BitWidth;
        uint8_t bit1 : BitWidth;
        uint8_t bit2 : BitWidth;
        uint8_t bit3 : BitWidth;
        uint8_t bit4 : BitWidth;
        uint8_t bit5 : BitWidth;
        uint8_t bit6 : BitWidth;
        uint8_t bit7 : BitWidth;
    };
    uint8_t byte;
};

union AtomicBytes {
    struct {
        std::atomic<uint8_t> bit0;
        std::atomic<uint8_t> bit1;
        std::atomic<uint8_t> bit2;
        std::atomic<uint8_t> bit3;
        std::atomic<uint8_t> bit4;
        std::atomic<uint8_t> bit5;
        std::atomic<uint8_t> bit6;
        std::atomic<uint8_t> bit7;
    } bytes;
    std::atomic<uint8_t> byte;
};

union MaskUint64 {
    struct {
        uint64_t byte0 : ByteWidth;
        uint64_t byte1 : ByteWidth;
        uint64_t byte2 : ByteWidth;
        uint64_t byte3 : ByteWidth;
        uint64_t byte4 : ByteWidth;
        uint64_t byte5 : ByteWidth;
        uint64_t byte6 : ByteWidth;
        uint64_t byte7 : ByteWidth;
    } bytes;
    uint64_t dword;
};

union AtomicMaskUint64 {
    struct {
        std::atomic<uint8_t> byte0;
        std::atomic<uint8_t> byte1;
        std::atomic<uint8_t> byte2;
        std::atomic<uint8_t> byte3;
        std::atomic<uint8_t> byte4;
        std::atomic<uint8_t> byte5;
        std::atomic<uint8_t> byte6;
        std::atomic<uint8_t> byte7;
    } bytes;
    std::atomic<uint64_t> dword;
};

union AtomicAlignasMaskUint64 {
    struct {
        alignas(128)std::atomic<uint8_t> byte0;
        alignas(128)std::atomic<uint8_t> byte1;
        alignas(128)std::atomic<uint8_t> byte2;
        alignas(128)std::atomic<uint8_t> byte3;
        alignas(128)std::atomic<uint8_t> byte4;
        alignas(128)std::atomic<uint8_t> byte5;
        alignas(128)std::atomic<uint8_t> byte6;
        alignas(128)std::atomic<uint8_t> byte7;
    } bytes;
    std::atomic<uint64_t> dword;
};

union MaskRByte {
    uint8_t byte;
    struct {
        uint8_t bit0 : BitWidth;
        uint8_t bit1 : BitWidth;
        uint8_t bit2 : BitWidth;
        uint8_t bit3 : BitWidth;
        uint8_t bit4 : BitWidth;
        uint8_t bit5 : BitWidth;
        uint8_t bit6 : BitWidth;
        uint8_t bit7 : BitWidth;
    };
};

enum MaskPtrBitType {
    OffsetBegining = 0,
    OffsetAddress = 47,
    OffsetControl0 = 48,
    OffsetControl1 = 49,
    OffsetControl2 = 50,
    OffsetControl3 = 51,
    OffsetControl4 = 52,
    OffsetControl5 = 53,
    OffsetControl6 = 54,
    OffsetControl7 = 55,
    OffsetControl8 = 56,
    OffsetControl9 = 57,
    OffsetControl10 = 58,
    OffsetControl11 = 59,
    OffsetControl12 = 60,
    OffsetControl13 = 61,
    OffsetControl14 = 62,
    OffsetControl15 = 63,
    OffsetTerminate = 64
};

union MaskRWPtr {
    struct {
        uint64_t rAddress : OffsetControl0 - OffsetBegining; // 6 Bytes
        uint64_t control0 : OffsetControl1 - OffsetControl0; // BitWidth
        uint64_t control1 : OffsetControl2 - OffsetControl1; // BitWidth
        uint64_t control2 : OffsetControl3 - OffsetControl2; // BitWidth
        uint64_t control3 : OffsetControl4 - OffsetControl3; // BitWidth
        uint64_t control4 : OffsetControl5 - OffsetControl4; // BitWidth
        uint64_t control5 : OffsetControl6 - OffsetControl5; // BitWidth
        uint64_t control6 : OffsetControl7 - OffsetControl6; // BitWidth
        uint64_t control7 : OffsetControl8 - OffsetControl7; // BitWidth
        uint64_t control8 : OffsetControl9 - OffsetControl8; // BitWidth
        uint64_t control9 : OffsetControl10 - OffsetControl9; // BitWidth
        uint64_t control10 : OffsetControl11 - OffsetControl10; // BitWidth
        uint64_t control11 : OffsetControl12 - OffsetControl11; // BitWidth
        uint64_t control12 : OffsetControl13 - OffsetControl12; // BitWidth
        uint64_t control13 : OffsetControl14 - OffsetControl13; // BitWidth
        uint64_t control14 : OffsetControl15 - OffsetControl14; // BitWidth
        uint64_t control15 : OffsetTerminate - OffsetControl15; // BitWidth
    };
    struct {
        uint64_t wAddress : OffsetControl0 - OffsetBegining; // 6 Bytes
        uint64_t wControl : OffsetTerminate - OffsetControl0; // 2 Bytes or short
    };
    uint64_t rwptr;
};


#ifdef __cplusplus
extern "C" {
#endif
uint64_t atomic_add(uint64_t *refv, uint64_t value) {
    return __sync_add_and_fetch(refv, value);
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
uint64_t atomic_mask_add(MaskUint64 *refv, int idx, uint64_t value) {
    return __sync_fetch_and_add((volatile uint8_t *) (((uint8_t *) &refv->bytes) + idx), (uint8_t) value);
}
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
uint64_t atomic_mask_sub(MaskUint64 *refv, int idx, uint64_t value) {
    return __sync_fetch_and_sub((volatile uint8_t *) (((uint8_t *) &refv->bytes) + idx), (uint8_t) value);
}
#ifdef __cplusplus
}
#endif

#endif //HASHCOMP_MASKRWPTR_H
