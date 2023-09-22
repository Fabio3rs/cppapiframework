#include "../src/utils/CLog.hpp"
#include "../src/utils/LogUtils.hpp"
#include "../src/utils/ProcessHelper.hpp"
#include "allocation_count.hpp"
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <ratio>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>
#include <utility>

// NOLINTNEXTLINE
TEST(TestLog, Open) {
    auto &log = CLog::initSingleton("TestLog.log");

    for (size_t i = 0; i < 100; i++) {
        EXPECT_FALSE(log.multiRegister("Teste log %0", i).empty());
    }
}

// NOLINTNEXTLINE
TEST(TestLog, MultiRegisterStrEqual) {
    auto &log = CLog::initSingleton("TestLogStrEqual.log");

    EXPECT_EQ(log.multiRegister("Teste log (%0) AAAAAAAAAAAA", 10),
              "Teste log (10) AAAAAAAAAAAA");
    EXPECT_EQ(log.multiRegister("Teste log (%0) AAAAAAAAAAAA %1 | %2", 10,
                                "Teste", 20),
              "Teste log (10) AAAAAAAAAAAA Teste | 20");
}

// NOLINTNEXTLINE
TEST(TestLog, MultiRegisterAllocationLimit) {
    auto &log = CLog::initSingleton("TestLogAllocations.log");

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    AllocationCount::getAllocationCount() = 0;
    log.multiRegister("Teste log (%0) AAAAAAAAAAAA", 10);
    EXPECT_LE(AllocationCount::getAllocationCount(), 3);
}

static void somelogadd() {
    auto &log = CLog::initSingleton();

    for (size_t i = 0; i < 50000; i++) {
        log.multiRegister("Teste log %0", i);
    }
}

// NOLINTNEXTLINE
TEST(TestLog, SaturateLog) {
    std::filesystem::remove("Saturate.log");
    auto &log = CLog::initSingleton("Saturate.log");

    for (size_t i = 0; i < 100; i++) {
        EXPECT_FALSE(log.multiRegister("Teste log %0", i).empty());
    }

    size_t THREADS = 8;

    std::vector<std::thread> mthreads;
    mthreads.reserve(THREADS);

    for (size_t i = 0; i < THREADS; i++) {
        mthreads.emplace_back(somelogadd);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    for (auto &thr : mthreads) {
        if (thr.joinable()) {
            thr.join();
        }
    }

    log.FinishLog();
    std::filesystem::remove("Saturate.log");
}

// NOLINTNEXTLINE
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
        auto &log = CLog::initSingleton(CLog::defaultcfg);

        for (size_t i = 0; i < 100; i++) {
            EXPECT_FALSE(log.multiRegister("Teste log %0", i).empty());
        }

        log.FinishLog();
    }

    redirectionlog.seekg(0, std::ios::end);
    EXPECT_GT(redirectionlog.tellg(), 2000);
}

static void logDebugInsertFork() {
    auto &log = CLog::log();
    log.multiRegister("Before fork");

    ProcessHelper phelper;

    auto forked = phelper.fork();

    if (forked == 0) {
        log.multiRegister("Inside fork");
        log.multiRegister("Will exit fork");
        log.FinishLog();
        exit(0);
    } else if (forked > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        log.multiRegister("Inside parent");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        phelper.wait(forked, 0);
    } else {
        throw std::runtime_error("Error fork");
    }

    log.multiRegister("After fork");
    log.FinishLog();
}

static auto findLineOnStream(std::fstream &toSearch, const std::string &val)
    -> bool {
    toSearch.seekg(0);

    std::string line;

    while (std::getline(toSearch, line)) {
        std::cout << line << std::endl;
        std::size_t posFound = line.find(val);

        if (posFound != std::string::npos) {
            return true;
        }
    }

    return false;
}

// NOLINTNEXTLINE
TEST(TestLog, ForkAndLog) {
    signal(SIGCHLD, SIG_IGN);
    std::fstream fsout("ForkAndLog.log",
                       std::ios::trunc | std::ios::out | std::ios::in);

    CLog::defaultcfg.filename.clear();
    CLog::defaultcfg.stream = &fsout;
    CLog::initSingleton(CLog::defaultcfg).multiRegister("Before test");
    logDebugInsertFork();

    fsout.flush();

    EXPECT_TRUE(findLineOnStream(fsout, "Inside fork"));
    EXPECT_TRUE(findLineOnStream(fsout, "Will exit fork"));
    EXPECT_TRUE(findLineOnStream(fsout, "Inside fork"));
}
