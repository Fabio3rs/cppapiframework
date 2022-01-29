#include "CLog.hpp"
#include "ChronoUtils.hpp"
#include "CircleMTIO.hpp"
#include <ctime>
#include <exception>
#include <iostream>

#ifndef LOG_FILE_NAME_PATH
#define LOG_FILE_NAME_PATH "program.log"
#endif

#ifndef FULL_COMMIT_HASH
#define FULL_COMMIT_HASH "#"
#endif

namespace {
struct LogLine {
    std::string line;
    std::thread::id thrid;
    std::chrono::high_resolution_clock::time_point when;
};
} // namespace

using logCircleIo_t = CircleMTIO<512, LogLine>;
static std::unique_ptr<logCircleIo_t> logLinesBuffer(std::make_unique<logCircleIo_t>());

auto CLog::addLinesToLog(CLog &logInst) -> bool {
    bool continueRunning = true;
    bool shouldFlush = false;
    while (continueRunning) {
        auto nextLine = logLinesBuffer->next();

        if (nextLine.first != nullptr && nextLine.second) {
            const auto &lineInf = *nextLine.first;
            const auto &line = nextLine.first->line;

            auto tempo = ChronoUtils::GetDateAndTime(lineInf.when);

            logInst.LogFile << tempo;
            logInst.LogFile << ": ";
            logInst.LogFile << line;
            logInst.LogFile << "\n";

            shouldFlush = true;
        } else {
            continueRunning = false;
        }
    }

    return shouldFlush;
}

void CLog::threadFn(CLog &logInst) {
    bool keepRunning = true;
    while (keepRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        if (!logInst.running) {
            keepRunning = false;
        }

        if (addLinesToLog(logInst)) {
            logInst.LogFile.flush();
        }
    }

    logInst.LogFile.flush();
}

auto CLog::log(const char *filepath) -> CLog & {
    static CLog Log(filepath != nullptr ? filepath : LOG_FILE_NAME_PATH);
    return Log;
}

CLog::CLog(const std::string &NameOfFile) {
    Finished = false;
    running = true;
    std::cout << "Iniciando log em " << NameOfFile << std::endl;
    LogFile.open(NameOfFile, std::ios::out | std::ios::app);

    if (!LogFile.good()) {
        throw std::runtime_error("Impossivel criar ou abrir o arquivo de log");
    }

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

    writterThreadInst = std::thread(threadFn, std::ref(*this));
}

CLog::~CLog() noexcept { FinishLog(); }

void CLog::AddToLog(const std::string &Text, const std::string &extraid) {
    auto logLine = logLinesBuffer->new_write();

    if (logLine.first == nullptr) {
        throw std::runtime_error("Log failed");
    }

    std::string &Temp = logLine.first->line;
    Temp.reserve(Text.size() + extraid.size());
    logLine.first->when = std::chrono::high_resolution_clock::now();
    logLine.first->thrid = std::this_thread::get_id();

    Temp += extraid;
    Temp += " ";
    Temp += Text;

    logLinesBuffer->set_ready(logLine.second);
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
}
