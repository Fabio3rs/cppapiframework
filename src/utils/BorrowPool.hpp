#pragma once
#include <cstddef>
#include <stdexcept>
#ifndef UTILS_BORROWPOOL_HPP
#define UTILS_BORROWPOOL_HPP
/**
 * @file BorrowPool.hpp
 * @author Fabio R. Sluzala ()
 * @brief Pool para manter objetos alocados e emprestar os recursos para threads
 * que o necessitem https://github.com/Fabio3rs/BorrowPool
 * @version 0.1
 * @date 2021-12-10
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <atomic>
#include <bitset>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

#ifndef CPU_COMMON_CACHELINE_SIZE
#define CPU_COMMON_CACHELINE_SIZE 64
#endif

template <class T> class BorrowPool;

template <class T> struct cachelinevar {
    T a;
} __attribute__((aligned(CPU_COMMON_CACHELINE_SIZE)));

template <class T> class BorrowedObject {
    friend BorrowPool<T>;
    BorrowPool<T> *originpool{nullptr};
    cachelinevar<T> *bobject{nullptr};
    size_t pos{0};

    BorrowedObject() = default;

  public:
    auto operator->() -> T * { return &bobject->a; }
    auto operator->() const -> const T * { return &bobject->a; }

    auto operator*() -> T & { return bobject->a; }
    auto operator*() const -> const T & { return *bobject->a; }

    [[nodiscard]] auto get() -> T * { return &bobject->a; }
    [[nodiscard]] auto get() const -> const T * { return &bobject->a; }

    auto getId() const -> size_t { return pos; }

    explicit operator bool() const { return bobject != nullptr; }

    auto operator=(const BorrowedObject &) -> BorrowedObject & = delete;

    auto operator=(BorrowedObject &&n) noexcept -> BorrowedObject & {
        if (std::addressof(n) == this) {
            return *this;
        }

        originpool = n.originpool;
        bobject = n.bobject;
        pos = n.pos;

        n.originpool = nullptr;
        n.bobject = nullptr;

        return *this;
    }

    BorrowedObject(const BorrowedObject &) = delete;

    BorrowedObject(BorrowedObject &&n) noexcept { *this = std::move(n); }

    ~BorrowedObject() {
        if (originpool && bobject) {
            originpool->disownBorrowed(*this);
        }
    }
};

template <class poolobject_t> class BorrowPool {
    friend BorrowedObject<poolobject_t>;
    
    std::atomic<size_t> firstFreeElement;
    std::atomic<size_t> borrowedElements;

    std::deque<std::atomic<bool>> bits;

    std::deque<cachelinevar<poolobject_t>> tmp;

    std::mutex waitmtx;
    std::condition_variable waitcmd;

    void disownBorrowed(BorrowedObject<poolobject_t> &borrowed) {
        bits[borrowed.pos] = false;
        firstFreeElement = borrowed.pos;

        --borrowedElements;

        waitcmd.notify_one();
    }

    [[nodiscard]] auto findFreeElement() const -> size_t {
        size_t result = INVALIDPOS;
        for (size_t i = 0, size = bits.size(); i < size; ++i) {
            if (!bits[i]) {
                return i;
            }
        }

        return result;
    }

    auto getValidPos() -> size_t {
        size_t result = INVALIDPOS;

        result = firstFreeElement.exchange(INVALIDPOS);

        if (result != INVALIDPOS) {
            bool val = false;
            if (!bits[result].compare_exchange_strong(val, true)) {
                result = INVALIDPOS;
            } else {
                ++borrowedElements;
                return result;
            }
        }

        if (borrowedElements == bits.size()) {
            return INVALIDPOS;
        }

        result = INVALIDPOS;

        for (size_t i = 0, size = bits.size(); i < size; ++i) {
            bool val = false;
            if (bits[i].compare_exchange_strong(val, true)) {
                result = i;
                break;
            }
        }

        if (result != INVALIDPOS) {
            ++borrowedElements;
        }

        return result;
    }

  public:
    constexpr static std::size_t INVALIDPOS{~std::size_t(0UL)};

    auto borrow(std::chrono::seconds timeout = std::chrono::hours(2))
        -> BorrowedObject<poolobject_t> {
        std::size_t pos = getValidPos();

        while (pos == INVALIDPOS) {
            std::unique_lock<std::mutex> lck(waitmtx);

            if (waitcmd.wait_for(lck, timeout) == std::cv_status::no_timeout) {

                pos = getValidPos();

                if (pos == INVALIDPOS) {
                    continue;
                }
            } else {
                BorrowedObject<poolobject_t> nobject;

                nobject.pos = INVALIDPOS;
                nobject.originpool = nullptr;
                nobject.bobject = nullptr;

                return nobject;
            }
        }

        BorrowedObject<poolobject_t> nobject;

        nobject.pos = pos;
        nobject.originpool = this;
        nobject.bobject = &tmp[pos];

        return nobject;
    }

    explicit BorrowPool(size_t ELEMENTS) {
        firstFreeElement = 0;
        borrowedElements = 0;
        tmp.resize(ELEMENTS);
        for (size_t i = 0; i < ELEMENTS; i++) {
            bits.emplace_back(false);
        }
    }
};

#endif
