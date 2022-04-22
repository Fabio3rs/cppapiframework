#include "CLog.hpp"
#include "ChronoUtils.hpp"
#include "CircleMTIO.hpp"
#include "ScopedStreamRedirect.hpp"
#include <cstddef>
#include <ctime>
#include <exception>
#include <iostream>
#include <memory>
#include <thread>

#ifndef LOG_FILE_NAME_PATH
#define LOG_FILE_NAME_PATH "program.log"
#endif

#ifndef FULL_COMMIT_HASH
#define FULL_COMMIT_HASH "#"
#endif

namespace {
struct __attribute__((aligned(64))) LogLine {
    std::string line;
    std::thread::id thrid;
    std::chrono::high_resolution_clock::time_point when;
};
} // namespace

using logCircleIo_t = CircleMTIO<1024, LogLine>;
static std::unique_ptr<logCircleIo_t>
    logLinesBuffer(std::make_unique<logCircleIo_t>());

static std::unique_ptr<CLog> logInstance;

auto CLog::addLinesToLog(CLog &logInst) -> bool {
    bool continueRunning = true;
    bool shouldFlush = false;
    while (continueRunning) {
        auto nextLine = logLinesBuffer->next();

        if (nextLine.first != nullptr) {
            const auto &lineInf = *nextLine.first;
            const auto &line = nextLine.first->line;

            auto tempo = ChronoUtils::GetDateAndTime(lineInf.when);

            logInst.LogFile << tempo;
            logInst.LogFile << ": ";
            logInst.LogFile << lineInf.thrid;
            logInst.LogFile << " ";
            logInst.LogFile << line;
            logInst.LogFile << "\n";

            shouldFlush = true;

            logLinesBuffer->set_free(nextLine.second);
        } else {
            continueRunning = false;
        }
    }

    return shouldFlush;
}

void CLog::threadFn(CLog &logInst) {
    bool keepRunning = true;
    int WAIT_COUNT = 5;
    while (keepRunning) {
        if (WAIT_COUNT > 4) {
            std::unique_lock<std::mutex> lck(logInst.cmdmtx);
            if (logInst.waitcmd.wait_for(
                    lck, std::chrono::milliseconds(WAIT_COUNT)) !=
                std::cv_status::timeout) {
                WAIT_COUNT = 0;
            }
        } else {
            int tmp = std::max(5, ++WAIT_COUNT);
            WAIT_COUNT = tmp;
        }

        if (!logInst.running) {
            keepRunning = false;
        }

        if (addLinesToLog(logInst)) {
            logInst.LogFile.flush();
        }
    }

    logInst.LogFile.flush();
}

logOutputInfo CLog::defaultcfg{LOG_FILE_NAME_PATH, nullptr};

auto CLog::log() -> CLog & { return *logInstance; }

auto CLog::initSingleton(const logOutputInfo &logcfg) -> CLog & {
    if (!logInstance) {
        logInstance = std::make_unique<CLog>(logcfg);
    }

    return *logInstance;
}

auto CLog::initSingleton(std::string_view filepath) -> CLog & {
    if (!logInstance) {
        defaultcfg.filename = filepath;
        defaultcfg.stream = nullptr;
        logInstance = std::make_unique<CLog>(defaultcfg);
    }

    return *logInstance;
}

void CLog::insertLogHeader() {
    LogFile << "***********************************************************"
               "******************\n";
    LogFile << std::string("* Program compilation date/time: ") + __DATE__ +
                   " " + __TIME__;
    LogFile << "\n*********************************************************"
               "********************\n";
    LogFile << "* Full commit hash: " FULL_COMMIT_HASH;
    LogFile << "\n*********************************************************"
               "********************\n";
    LogFile << "***********************************************************"
               "******************\n* Log started at: ";
    LogFile << ChronoUtils::GetDateAndTime();
    LogFile << "\n*********************************************************"
               "********************\n\n";
    LogFile.flush();
}

CLog::CLog(const logOutputInfo &logcfg) : running(true) {
    Finished = false;
    running = true;

    if (logcfg.stream != nullptr) {
        streamRedirect =
            std::make_unique<ScopedStreamRedirect>(LogFile, *logcfg.stream);
    } else {
        std::cout << "Iniciando log em " << logcfg.filename << std::endl;
        LogFile.open(logcfg.filename, std::ios::out | std::ios::app);

        if (!LogFile.good()) {
            throw std::runtime_error(
                "Impossivel criar ou abrir o arquivo de log");
        }
    }

    insertLogHeader();

    writterThreadInst = std::thread(threadFn, std::ref(*this));
}

CLog::~CLog() noexcept { FinishLog(); }

void CLog::AddToLog(const std::string &Text, const std::string &extraid) {
    std::pair<LogLine *, size_t> logLine;

    do {
        logLine = logLinesBuffer->new_write();
        if (logLine.first == nullptr) {
            waitcmd.notify_one();
            std::this_thread::sleep_for(std::chrono::nanoseconds(100));
        }
    } while (logLine.first == nullptr);

    std::string &Temp = logLine.first->line;
    Temp.clear();
    Temp.reserve(Text.size() + extraid.size());
    logLine.first->when = std::chrono::high_resolution_clock::now();
    logLine.first->thrid = std::this_thread::get_id();

    Temp += extraid;
    Temp += " ";
    Temp += Text;

    logLinesBuffer->set_ready(logLine.second);
}

void CLog::PrepareToFork() {
    running = false;

    if (writterThreadInst.joinable()) {
        writterThreadInst.join();
    }

    writterThreadInst = std::thread();
    running = true;
}

void CLog::SignalFork() {
    logLinesBuffer.reset();
    logLinesBuffer = std::make_unique<logCircleIo_t>();
    writterThreadInst = std::thread(threadFn, std::ref(*this));
}

void CLog::ParentPostFork() {
    writterThreadInst = std::thread(threadFn, std::ref(*this));
}

void CLog::FinishLog() {
    std::this_thread::yield();
    running = false;

    if (writterThreadInst.joinable()) {
        writterThreadInst.join();
    }

    std::string LogContents;
    LogContents += "\n*********************************************************"
                   "********************\n* Log Finished at: ";
    LogContents += ChronoUtils::GetDateAndTime();
    LogContents += "\n*********************************************************"
                   "********************\n";
    LogFile.clear();

    LogFile.write(LogContents.c_str(),
                  static_cast<std::streamsize>(LogContents.size()));
    LogFile.flush();
    Finished = true;
    streamRedirect.reset();
}
