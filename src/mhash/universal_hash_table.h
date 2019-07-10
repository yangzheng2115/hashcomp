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
#include <boost/lockfree/stack.hpp>

namespace neatlib {
template<class Key, class T, class Hash = std::hash<Key>,
        std::size_t HASH_LEVEL = DEFAULT_NEATLIB_HASH_LEVEL,
        std::size_t ROOT_HASH_LEVEL = DEFAULT_NEATLIB_HASH_LEVEL,
        template<typename/* A, typename D = std::default_delete<A>*/> class ACCESSOR = /*std::unique_ptr*/boost::atomic_shared_ptr,
        template<typename, typename ...> class STACK= /*std::stack*/boost::lockfree::stack>
class UniversalHashTable {
private:
    enum node_type {
        DATA_NODE = 0, ARRAY_NODE
    };

    template<typename N> using CHEAPER_ACCESSOR = boost::shared_ptr<N>;
    //template<typename S> using STACK = /*std::stack*/boost::lockfree::stack<S>;

    constexpr static std::size_t ARRAY_SIZE = static_cast<const std::size_t>(get_power2<HASH_LEVEL>::value);
    constexpr static std::size_t ROOT_ARRAY_SIZE = static_cast<const std::size_t>(get_power2<ROOT_HASH_LEVEL>::value);

private:
    struct node {
        node_type type_;

        explicit node(node_type type) : type_(type) {}
    };

    struct data_node : node {
        const std::pair<const Key, const T> data_;
        const std::size_t hash_;

        data_node(const Key &key, const T &mapped) : node(node_type::DATA_NODE), data_(key, mapped),
                                                     hash_(Hash()(key)) {}

        data_node(const Key &key, const T &mapped, const std::size_t h) : node(node_type::DATA_NODE),
                                                                          data_(key, mapped), hash_(h) {}

        std::size_t hash() const { return hash_; }
    };

    struct array_node : node {
        std::array<ACCESSOR<node>, ARRAY_SIZE> arr_;

        array_node() : node(node_type::ARRAY_NODE), arr_() {}

        constexpr static std::size_t size() { return ARRAY_SIZE; }
    };

    struct reserved_pool {
        STACK<ACCESSOR<data_node>> data_st_;

        reserved_pool() : data_st_() {}

        void put() {
            data_st_.push(ACCESSOR<data_node>());
        }

        void put(std::size_t sz) {
            for (std::size_t i = 0; i < sz; i++) put();
        }

        ACCESSOR<data_node> get_data_node(const Key &key, const T &mapped, std::size_t hash) {
            ACCESSOR<data_node> p(nullptr);
            data_st_.pop(p);
            p->data_.first = key;
            p->data_.second = mapped;
            p->hash_ = hash;
            return std::move(p);
        }
    };

    struct locator {
        CHEAPER_ACCESSOR<node> *loc_ref = nullptr;

        std::size_t level_hash(std::size_t hash, std::size_t level) {
            hash >>= ROOT_HASH_LEVEL;
            level--;
            return util::level_hash<Key>(hash, level, ARRAY_SIZE, HASH_LEVEL);
        }

        const Key &key() {
            return static_cast<data_node *>(loc_ref->get())->data_.first;
        }

        const std::pair<const Key, const T> &value() {
            return static_cast<data_node *>(loc_ref->get())->data_;
        }
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
