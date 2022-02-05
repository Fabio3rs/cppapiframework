#include "../src/utils/CLog.hpp"
#include "../src/utils/LogUtils.hpp"
#include "../src/utils/ProcessHelper.hpp"
#include "allocation_count.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <ratio>
#include <stdexcept>
#include <thread>
#include <unistd.h>

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestLog, Open) {
    auto &log = CLog::log("TestLog.log");

    for (size_t i = 0; i < 100; i++) {
        EXPECT_FALSE(log.multiRegister("Teste log %0", i).empty());
    }
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestLog, MultiRegisterStrEqual) {
    auto &log = CLog::log("TestLogStrEqual.log");

    EXPECT_EQ(log.multiRegister("Teste log (%0) AAAAAAAAAAAA", 10),
              "Teste log (10) AAAAAAAAAAAA");
    EXPECT_EQ(log.multiRegister("Teste log (%0) AAAAAAAAAAAA %1 | %2", 10,
                                "Teste", 20),
              "Teste log (10) AAAAAAAAAAAA Teste | 20");
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestLog, MultiRegisterAllocationLimit) {
    auto &log = CLog::log("TestLogAllocations.log");

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    AllocationCount::getAllocationCount() = 0;
    log.multiRegister("Teste log (%0) AAAAAAAAAAAA", 10);
    EXPECT_LE(AllocationCount::getAllocationCount(), 3);
}

static void somelogadd() {
    auto &log = CLog::log();

    for (size_t i = 0; i < 50000; i++) {
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

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    for (auto &thr : mthreads) {
        if (thr.joinable()) {
            thr.join();
        }
    }

    log.FinishLog();
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestLog, RedirectLog) {
    std::filesystem::remove("Redirection.log");
    std::fstream redirectionlog("Redirection.log",
                                std::ios::out | std::ios::trunc);

    if (!redirectionlog.is_open()) {
        throw std::runtime_error("Falha ao abrir o arquivo de log de testes");
    }

    CLog::defaultcfg.filename.clear();
    CLog::defaultcfg.stream = &redirectionlog;

    {
        auto &log = CLog::log();

        for (size_t i = 0; i < 100; i++) {
            EXPECT_FALSE(log.multiRegister("Teste log %0", i).empty());
        }

        log.FinishLog();
    }

    redirectionlog.seekg(0, std::ios::end);
    EXPECT_GT(redirectionlog.tellg(), 2000);
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestLog, ForkAndLog) {
    CLog::defaultcfg.filename.clear();
    CLog::defaultcfg.stream = &std::cout;

    CLog::log().multiRegister("Before fork");

    ProcessHelper phelper;

    auto forked = phelper.fork();

    if (forked == 0) {
        std::cout << "Inside fork" << std::endl;
        CLog::log().multiRegister("Inside fork");
    } else if (forked > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        CLog::log().multiRegister("Inside parent");
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    } else {
        throw std::runtime_error("Error fork");
    }

    CLog::log().multiRegister("After fork");

    EXPECT_TRUE(true);
}
