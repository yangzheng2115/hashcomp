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
#include <c++/5/utility>

#define COARSE_RESERVIOR 1

namespace neatlib {
class ThreadRegistry;

extern ThreadRegistry gThreadRegistry;

struct ThreadCheckInCheckOut;

extern thread_local ThreadCheckInCheckOut tl_tcico;

extern void thread_registry_deregister_thread(const int tid);

struct ThreadCheckInCheckOut {
    static const int NOT_ASSIGNED = -1;
    int tid{NOT_ASSIGNED};

    ~ThreadCheckInCheckOut() {
        if (tid == NOT_ASSIGNED) return;
        thread_registry_deregister_thread(tid);
    }
};

class ThreadRegistry {
private:
    alignas(128) std::atomic<bool> usedTID[REGISTRY_MAX_THREADS];   // Which TIDs are in use by threads
    alignas(128) std::atomic<int> maxTid{-1};                     // Highest TID (+1) in use by threads

public:
    ThreadRegistry() {
        for (int it = 0; it < REGISTRY_MAX_THREADS; it++) {
            usedTID[it].store(false, std::memory_order_relaxed);
        }
    }

    // Progress condition: wait-free bounded (by the number of threads)
    int register_thread_new(void) {
        for (int tid = 0; tid < REGISTRY_MAX_THREADS; tid++) {
            if (usedTID[tid].load(std::memory_order_acquire)) continue;
            bool unused = false;
            if (!usedTID[tid].compare_exchange_strong(unused, true)) continue;
            // Increase the current maximum to cover our thread id
            int curMax = maxTid.load();
            while (curMax <= tid) {
                maxTid.compare_exchange_strong(curMax, tid + 1);
                curMax = maxTid.load();
            }
            tl_tcico.tid = tid;
            return tid;
        }
        std::cout << "ERROR: Too many threads, registry can only hold " << REGISTRY_MAX_THREADS << " threads\n";
        assert(false);
    }

    // Progress condition: wait-free population oblivious
    inline void deregister_thread(const int tid) {
        usedTID[tid].store(false, std::memory_order_release);
    }

    // Progress condition: wait-free population oblivious
    static inline uint64_t getMaxThreads(void) {
        return gThreadRegistry.maxTid.load(std::memory_order_acquire);
    }

    // Progress condition: wait-free bounded (by the number of threads)
    static inline int getTID(void) {
        int tid = tl_tcico.tid;
        if (tid != ThreadCheckInCheckOut::NOT_ASSIGNED) return tid;
        //std::cout << "Thread " << gThreadRegistry.getMaxThreads() + 1 << " registered" << endl;
        return gThreadRegistry.register_thread_new();
    }
};

void thread_registry_deregister_thread(const int tid) {
    gThreadRegistry.deregister_thread(tid);
}

thread_local ThreadCheckInCheckOut tl_tcico{};

ThreadRegistry gThreadRegistry{};

template<class Key, class T, class Hash = std::hash<Key>,
        std::size_t HASH_LEVEL = DEFAULT_NEATLIB_HASH_LEVEL,
        std::size_t ROOT_HASH_LEVEL = DEFAULT_NEATLIB_HASH_LEVEL,
        template<typename, typename ...> class ACCESSOR = /*std::unique_ptr*/boost::atomic_shared_ptr,
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
    class node {
    protected:
        node_type type_;
    public:
        explicit node(node_type type) : type_(type) {}
    };

public:
    class data_node : node {
        std::pair<Key, T> data_;
        std::size_t hash_;
    public:
        data_node(const Key &key, const T &mapped) : node(node_type::DATA_NODE), data_(key, mapped),
                                                     hash_(Hash()(key)) {}

        data_node(const Key &key, const T &mapped, const std::size_t h) : node(node_type::DATA_NODE),
                                                                          data_(key, mapped), hash_(h) {}

        ~data_node() {}

        void reset(const Key &key, const T &mapped) {
            this->type_ = node_type::DATA_NODE;
            //data_.first = key;
            //data_.second = mapped;
            data_ = std::pair<Key, T>(key, mapped);
            hash_ = Hash()(key);
        }

        //const std::pair<const Key, const T> &get() { return data_; }
        const std::pair<Key, T> &get() { return data_; }

        std::size_t hash() const { return hash_; }
    };

    data_node *reserviors[REGISTRY_MAX_THREADS];

    size_t thread_reservior_cursor[REGISTRY_MAX_THREADS];

private:
    class array_node : node {
        std::array<ACCESSOR<node>, ARRAY_SIZE> arr_;
    public:
        array_node() : node(node_type::ARRAY_NODE), arr_() {}

        constexpr static std::size_t size() { return ARRAY_SIZE; }
    };

    class reserved_pool {
        STACK<ACCESSOR<data_node>> data_st_;
    public:
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

    class locator {
        CHEAPER_ACCESSOR<node> *loc_ref = nullptr;

        std::size_t root_hash(std::size_t hash) { return hash & (ROOT_ARRAY_SIZE - 1); }

        std::size_t level_hash(std::size_t hash, std::size_t level) {
            hash >>= ROOT_HASH_LEVEL;
            level--;
            return util::level_hash<Key>(hash, level, ARRAY_SIZE, HASH_LEVEL);
        }

    public:
        const Key &key() { return static_cast<data_node *>(loc_ref->get())->data_.first; }

        const std::pair<const Key, const T> &value() { return static_cast<data_node *>(loc_ref->get())->data_; }

        locator(UniversalHashTable &ht, const Key &key) {
            std::size_t hash = ht.hasher_(key);
            std::size_t level = 0;
            array_node *curr_node = nullptr;
            for (; level < ht.max_level_; level++) {
                std::size_t curr_hash = 0;
                if (level) {
                    curr_hash = level_hash(hash, level);
                    loc_ref = curr_node->arr_[curr_hash].load(std::memory_order_release);
                } else {
                    curr_hash = root_hash(hash);
                    loc_ref = ht.root_[curr_hash].load(std::memory_order_release);
                }

                if (!loc_ref->get()) {
                    break;
                } else if (loc_ref->get()->type_ == DATA_NODE) {
                    if (hash != static_cast<data_node *>(loc_ref->get())->hash_) {
                        loc_ref = nullptr;
                    }
                    break;
                } else {
                    curr_node = static_cast<array_node *>(loc_ref->get());
                }
            }
        }

        locator(UniversalHashTable &ht, const Key &key, const T *mappedp) {
            const int tid = ThreadRegistry::getTID();
            std::size_t hash = ht.hasher_(key);
            std::size_t level = 0;
            bool end = false;
            array_node *curr_node = nullptr;
            for (; level < ht.max_level_ && !end; level++) {
                std::size_t curr_hash = 0;
                ACCESSOR<node> *atomic_pos = nullptr;
                if (level) {
                    curr_hash = level_hash(hash, level);
                    atomic_pos = curr_node->arr_[curr_hash];
                    loc_ref = atomic_pos->load(std::memory_order_release);
                } else {
                    curr_hash = root_hash(hash);
                    atomic_pos = ht.root_[curr_hash];
                    loc_ref = atomic_pos->load(std::memory_order_release);
                }

                if (!loc_ref->get()) {
                    loc_ref = nullptr;
                    break;
                } else if (loc_ref->get()->type_ == DATA_NODE) {
                    // Update/Delete bottleneck here.
                    CHEAPER_ACCESSOR<node> tmp_ptr(nullptr);
                    if (!mappedp) {
                        while (atomic_pos->compare_exchange_strong(loc_ref, nullptr));
                    } else {
#if COARSE_RESERVIOR
                        if (ht.thread_reservior_cursor[tid] == ARRAY_SIZE) {
                            ht.reserviors[tid] = new data_node[ARRAY_SIZE];
                        }
                        tmp_ptr.reset(
                                static_cast<data_node *>(new(
                                        ht.reserviors[tid][ht.thread_reservior_cursor[tid]])data_node(key, *mappedp,
                                                                                                      hash)));
#else
                        tmp_ptr.reset(static_cast<node *>(new data_node(key, *mappedp, hash)));
#endif
                        atomic<T *> currentp = nullptr;
                        do {
                            currentp = static_cast<data_node *>(loc_ref->get())->data_.second;
                        } while (currentp.compare_exchange_strong(loc_ref->get()));
                    }
                } else {
                    curr_node = static_cast<array_node *>(loc_ref->get());
                }
            }
        }
    };

public:
    UniversalHashTable() : size_(0) {
        std::size_t m = 1, num = ARRAY_SIZE, level = 1;
        std::size_t total_bit = sizeof(Key) * 8;
        if (total_bit < 64) {
            for (std::size_t i = 0; i < total_bit; i++)
                m *= 2;
            for (; num < m; num += num * ARRAY_SIZE)
                level++;
        } else {
            m = std::numeric_limits<std::size_t>::max();
            auto m2 = m / 2;
            for (; num < m2; num += num * ARRAY_SIZE)
                level++;
            level++;
        }
        max_level_ = level;
        for (int i = 0; i < REGISTRY_MAX_THREADS; i++) {
            reserviors[i] = static_cast<data_node *>(malloc(sizeof(data_node) * ARRAY_SIZE));
            thread_reservior_cursor[i] = ARRAY_SIZE;
        }
    }

    std::pair<const Key, const T> Get(const Key &key) {
        return std::make_pair(static_cast<Key>(0), static_cast<T>(0));
    }

    bool Upsert(const Key &key, const T &new_mapped) {
        //cout << ThreadRegistry::getTID() << endl;
        ThreadRegistry::getTID();
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

    std::size_t Size() { return size_; }

private:
    std::array<ACCESSOR<node>, ROOT_ARRAY_SIZE> root_;
    std::atomic<size_t> size_;
    Hash hasher_;
    std::size_t max_level_ = 0;
};
}

#endif //HASHCOMP_UNIVERSAL_HASH_TABLE_H
