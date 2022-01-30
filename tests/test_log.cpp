#include "../src/utils/CLog.hpp"
#include <filesystem>
#include <gtest/gtest.h>
#include <unistd.h>

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestLog, Open) {
    auto &log = CLog::log("TestLog.log");

    for (size_t i = 0; i < 100; i++) {
        EXPECT_FALSE(log.multiRegister("Teste log %0", i).empty());
    }
}

static void somelogadd() {
    auto &log = CLog::log();

    for (size_t i = 0; i < 100000; i++) {
        log.multiRegister("Teste log %0", i);
    }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestLog, SaturateLog) {
    std::filesystem::remove("Saturate.log");
    auto &log = CLog::log("Saturate.log");

    for (size_t i = 0; i < 100; i++) {
        EXPECT_FALSE(log.multiRegister("Teste log %0", i).empty());
    }

    size_t THREADS = 8;

    std::vector<std::thread> mthreads;
    mthreads.reserve(THREADS);

    for (size_t i = 0; i < THREADS; i++) {
        mthreads.emplace_back(std::thread(somelogadd));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (auto &thr : mthreads) {
        if (thr.joinable()) {
            thr.join();
        }
    }

    log.FinishLog();
}
