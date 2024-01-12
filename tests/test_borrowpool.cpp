#include "projstdafx.hpp"

#include <gtest/gtest.h>

// NOLINTNEXTLINE
TEST(TestBorrowPool, IsConnected) {
    BorrowPool<uint64_t> pool(32);

    auto borrowed = pool.borrow();

    EXPECT_TRUE(borrowed);
}

static void TestBorrowPoolInt64(BorrowPool<uint64_t> &pool) {
    for (size_t i = 0; i < 100000; i++) {
        auto borrowed = pool.borrow();

        EXPECT_TRUE(borrowed);
        if (borrowed) {
            auto &value = *borrowed;
            ++value;

            std::this_thread::yield();
        } else {
            EXPECT_TRUE(false);
        }
    }
}

static void TestBorrowPoolString(BorrowPool<std::string> &pool) {
    for (size_t i = 0; i < 100000; i++) {
        auto borrowed = pool.borrow();

        EXPECT_TRUE(borrowed);
        if (borrowed) {
            auto &value = *borrowed;
            value.clear();
            value += "aaaaaa";
            value += "123456789";
            value += "AAAAAAAAAAAAAA";

            std::this_thread::yield();
        } else {
            EXPECT_TRUE(false);
        }
    }
}

// NOLINTNEXTLINE
TEST(TestBorrowPool, SaturatePoolRequestInteger) {
    BorrowPool<uint64_t> pool(32);
    size_t THREADS = 32;

    std::vector<std::thread> mthreads;
    mthreads.reserve(THREADS);

    for (size_t i = 0; i < THREADS; i++) {
        mthreads.emplace_back(TestBorrowPoolInt64, std::ref(pool));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (auto &thr : mthreads) {
        if (thr.joinable()) {
            thr.join();
        }
    }
}


// NOLINTNEXTLINE
TEST(TestBorrowPool, SaturatePoolRequestString) {
    BorrowPool<std::string> pool(32);
    size_t THREADS = 32;

    std::vector<std::thread> mthreads;
    mthreads.reserve(THREADS);

    for (size_t i = 0; i < THREADS; i++) {
        mthreads.emplace_back(TestBorrowPoolString, std::ref(pool));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (auto &thr : mthreads) {
        if (thr.joinable()) {
            thr.join();
        }
    }
}
