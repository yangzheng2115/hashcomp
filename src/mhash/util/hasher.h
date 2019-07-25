//
// Created by lwh on 19-7-24.
//

#ifndef HASHCOMP_HASHER_H
#define HASHCOMP_HASHER_H

#include "City.h"
#include "MurmurHash1.h"
#include "MurmurHash2.h"
#include "MurmurHash3.h"

#define MHASHER     0
#define MURMURHASH1 1
#define MURMURHASH2 2
#define MURMURHASH3 3
#define CITYHASH    4

#ifndef HASHFUNC
#define HASHFUNC    CITYHASH
#endif


template<typename T>
struct mhasher {
    inline uint64_t Rotr64(uint64_t x, std::size_t n) {
        return (((x) >> n) | ((x) << (64 - n)));
    }

    inline uint64_t GetHashCode(uint64_t input) {
        uint64_t local_rand = input;
        uint64_t local_rand_hash = 8;
        local_rand_hash = 40343 * local_rand_hash + ((local_rand) & 0xFFFF);
        local_rand_hash = 40343 * local_rand_hash + ((local_rand >> 16) & 0xFFFF);
        local_rand_hash = 40343 * local_rand_hash + ((local_rand >> 32) & 0xFFFF);
        local_rand_hash = 40343 * local_rand_hash + (local_rand >> 48);
        local_rand_hash = 40343 * local_rand_hash;
        return Rotr64(local_rand_hash, 43);
        /*Func<long, long> hash =
        e => 40343 * (40343 * (40343 * (40343 * (40343 * 8 + (long) ((e) & 0xFFFF)) + (long) ((e >> 16) & 0xFFFF)) +
                               (long) ((e >> 32) & 0xFFFF)) + (long) (e >> 48));*/
    }

    inline uint64_t HashBytes(const uint16_t *str, size_t len) {
        // 40343 is a "magic constant" that works well,
        // 38299 is another good value.
        // Both are primes and have a good distribution of bits.
        const uint64_t kMagicNum = 40343;
        uint64_t hashState = len;

        for (size_t idx = 0; idx < len; ++idx) {
            hashState = kMagicNum * hashState + str[idx];
        }

        // The final scrambling helps with short keys that vary only on the high order bits.
        // Low order bits are not always well distributed so shift them to the high end, where they'll
        // form part of the 14-bit tag.
        return Rotr64(kMagicNum * hashState, 6);
    }

    inline uint64_t HashBytes(const char *str) {
        const uint64_t kMagicNum = 40343;
        size_t length = std::strlen(str);
        uint64_t hashState = length;

        for (size_t idx = 0; idx < length; ++idx) {
            hashState = kMagicNum * hashState + str[idx];
        }

        return Rotr64(kMagicNum * hashState, 6);
    }

    inline uint64_t GetHashCodeMur1(uint64_t key) {
        uint64_t seed = 40343;
        const uint64_t *pk = &key;
        return MurmurHash1Aligned(pk, sizeof(uint64_t), seed); // 0xb6d99cf8
    }

    inline uint64_t GetHashCodeMur2(uint64_t key) {
        uint64_t seed = 40343;
        const uint64_t *pk = &key;
        return MurmurHash64B(pk, sizeof(uint64_t), seed);
    }

    inline uint64_t GetHashCodeMur3(uint64_t key) {
        uint64_t seed = 40343;
        uint64_t hash_otpt[2] = {0};
        const uint64_t *pk = &key;
        MurmurHash3_x64_128(pk, sizeof(uint64_t), seed, hash_otpt); // 0xb6d99cf8
        return *hash_otpt;
    }

    inline uint64_t GetHashCodeCityHash(uint64_t key) {
        uint64_t seed = 40343;
        uint64_t hash_otpt[2] = {0};
        const char *pk = (const char *) &key;
        return CityHash64WithSeed(pk, sizeof(uint64_t), 40343);
    }

    inline uint64_t HashBytesMur1(const char *str) {
        return MurmurHash1Aligned(str, std::strlen(str), 40343);
    }

    inline uint64_t HashBytesMur2(const char *str) {
        return MurmurHash64B(str, std::strlen(str), 40343);
    }

    inline uint64_t HashBytesMur3(const char *str) {
        uint64_t seed = 40343;
        uint64_t hash_otpt[2] = {0};
        MurmurHash3_x64_128(str, std::strlen(str), seed, hash_otpt);
        return *hash_otpt;
    }

    inline uint64_t HashBytesCityHash(const char *str) {
        return CityHash64WithSeed(str, std::strlen(str), 40343);
    }

public:
    uint64_t hash(T t) {
        /*string li = typeid(T).name();
        string ri = typeid(char *).name();*/

        bool eq = (typeid(T) == typeid(uint64_t));

        if (typeid(T) == typeid(const char *) || typeid(T) == typeid(char *)) {
#if (HASHFUNC == MHASHER)
            return HashBytes((const char *) t);
#elif (HASHFUNC == MURMURHASH1)
            return HashBytesMur1((const char *) t);
#elif (HASHFUNC == MURMURHASH2)
            return HashBytesMur2((const char *) t);
#elif (HASHFUNC == MURMURHASH3)
            return HashBytesMur3((const char *) t);
#elif (HASHFUNC == CITYHASH)
            return HashBytesCityHash((const char *) t);
#else
            return HashBytes((const char *) t);
#endif
        } else if (typeid(T) == typeid(uint64_t)) {
#if (HASHFUNC == MHASHER)
            return GetHashCode((uint64_t) t);
#elif (HASHFUNC == MURMURHASH1)
            return GetHashCodeMur1((uint64_t) t);
#elif (HASHFUNC == MURMURHASH2)
            return GetHashCodeMur2((uint64_t) t);
#elif (HASHFUNC == MURMURHASH3)
            return GetHashCodeMur3((uint64_t) t);
#elif (HASHFUNC == CITYHASH)
            return GetHashCodeCityHash((uint64_t) t);
#else
            return GetHashCode((uint64_t) t);
#endif
        }
        return static_cast<size_t >(-1);
    }

    uint64_t operator()(T t) {
        return hash(t);
    }
};

#endif //HASHCOMP_HASHER_H
