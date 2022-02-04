#pragma once
#ifndef CIRCLEMTIO_HPP
#define CIRCLEMTIO_HPP
#include <array>
#include <atomic>
#include <mutex>

template <class T> struct __attribute__((aligned(64))) singlevarnofsh { T a; };

template <size_t num, class T> class __attribute__((aligned(64))) CircleMTIO {
    std::array<T, num> elements;
    std::array<singlevarnofsh<std::atomic<int>>, num> elements_state;

    std::atomic<size_t> reading_point{};
    std::atomic<size_t> writing_point{};
    // std::atomic<size_t> read_point;
    std::mutex mtx;

  public:
    using value_type = T;
    using pair_t = std::pair<value_type, size_t>;

    auto new_write() -> std::pair<T *, size_t> {
        size_t a = writing_point.fetch_add(1);

        if (a >= num) {
            std::lock_guard<std::mutex> lck(mtx);
            a = writing_point.fetch_add(1);

            if (a >= num)
            {
                a = 0;
                writing_point = 1;
            }
        }

        if (elements_state[a].a != 0) {
            return std::pair<T *, size_t>(nullptr, 0);
        }

        elements_state[a].a = 1;

        return std::pair<T *, size_t>(&elements[a], a);
    }

    auto next() -> std::pair<T *, size_t> {
        size_t r = reading_point;

        if (elements_state[r].a != 2) {
            return std::pair<T *, size_t>(nullptr, ~std::size_t(0));
        }

        if (r == writing_point) {
            return std::pair<T *, size_t>(nullptr, ~std::size_t(0));
        }

        reading_point++;

        if (reading_point >= num) {
            reading_point = 0;
        }

        // read_point = r;

        return std::pair<T *, size_t>(&elements[r], r);
    }

    void set_free(size_t a) {
        elements_state[a].a = 0;
        // read_point = ~std::size_t(0);
    }

    void set_ready(size_t a) { elements_state[a].a = 2; }

    CircleMTIO() {
        reading_point = 0;
        writing_point = 0;
        // read_point = ~std::size_t(0);

        for (auto &b : elements_state) {
            b.a = 0;
        }
    }
};

#endif
