//
// Created by lwh on 19-7-9.
//

#ifndef HASHCOMP_UNIVERSAL_HASH_TABLE_H
#define HASHCOMP_UNIVERSAL_HASH_TABLE_H

#include <cassert>
#include <memory>
#include <cstddef>
#include <array>
#include <stack>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <functional>
#include "util.h"
#include <boost/smart_ptr/atomic_shared_ptr.hpp>

namespace neatlib {
template<class Key, class T, class Hash = std::hash<Key>,
        std::size_t HASH_LEVEL = DEFAULT_NEATLIB_HASH_LEVEL,
        std::size_t ROOT_HASH_LEVEL = DEFAULT_NEATLIB_HASH_LEVEL,
        template<typename N, typename D = default_delete<N>> class ACCESSOR = std::unique_ptr>
class UniversalHashTable {
private:
    enum node_type {
        DATA_NODE = 0, ARRAY_NODE
    };

    constexpr static std::size_t ARRAY_SIZE = static_cast<const std::size_t>(get_power2<HASH_LEVEL>::value);
    constexpr static std::size_t ROOT_ARRAY_SIZE = static_cast<const std::size_t>(get_power2<ROOT_HASH_LEVEL>::value);

private:
    class node {
        node_type type_;

        explicit node(node_type type) : type_(type) {}
    };

public:
    std::pair<const Key, const T> Get(const Key &key) {
        return std::make_pair(static_cast<Key>(0), static_cast<T>(0));
    }

    bool Upsert(const Key &key, const T &new_mapped) {
        return true;
    }

    bool Insert(const Key &key, const T &new_mapped) {
        return Upsert(key, new_mapped);
    }

    bool Update(const Key &key, const T &new_mapped) {
        return Upsert(key, new_mapped);
    }

    bool Remove(const Key &key) {
        return true;
    }

    std::size_t Size() {
        return size_;
    }

private:
    std::array<ACCESSOR<node>, ROOT_ARRAY_SIZE> root_;
    std::size_t size_;
};
}

#endif //HASHCOMP_UNIVERSAL_HASH_TABLE_H
