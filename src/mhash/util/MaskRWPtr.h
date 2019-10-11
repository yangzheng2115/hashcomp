//
// Created by iclab on 10/10/19.
//

#ifndef HASHCOMP_MASKRWPTR_H
#define HASHCOMP_MASKRWPTR_H

enum MaskPtrBitType {
    OffsetAddress = 0,
    OffsetSserdda = 47,
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
    OffsetControl15 = 63
};

union MaskRWPtr {
    struct {
        uint64_t rAddress : OffsetSserdda;
        uint64_t control0: OffsetControl0;
        uint64_t control1: OffsetControl1;
        uint64_t control2: OffsetControl2;
        uint64_t control3: OffsetControl3;
        uint64_t control4: OffsetControl4;
        uint64_t control5: OffsetControl5;
        uint64_t control6: OffsetControl6;
        uint64_t control7: OffsetControl7;
        uint64_t control8: OffsetControl8;
        uint64_t control9: OffsetControl9;
        uint64_t control10: OffsetControl10;
        uint64_t control11: OffsetControl11;
        uint64_t control12: OffsetControl12;
        uint64_t control13: OffsetControl13;
        uint64_t control14: OffsetControl14;
        uint64_t control15: OffsetControl15;
    };
    /*struct {
        uint64_t wAddress: OffsetSserdda;
        uint64_t wControl: short
    };*/
    uint64_t rwptr;
};

#endif //HASHCOMP_MASKRWPTR_H
