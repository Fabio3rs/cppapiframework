#pragma once
#include <ostream>
#include <utility>
#ifndef LOGGING_SYSTEM_CLOG_H
#define LOGGING_SYSTEM_CLOG_H
#include "ScopedStreamRedirect.hpp"
#include "StrFormat.hpp"
#include "Strutils.hpp"
#include <atomic>
#include <condition_variable>
#include <exception>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

struct logOutputInfo {
    std::string filename;
    std::ostream *stream{nullptr};
};

class CLog {

  public:
    ~CLog() noexcept;
    void AddToLog(const std::string &Text, const std::string &extraid = "");

    template <class... Types>
    auto multiRegister(std::string_view format, Types &&...args)
        -> std::string {
        std::string printbuf =
            StrFormat::multiRegister(format, std::forward<Types>(args)...);

        AddToLog(printbuf);
        return printbuf;
    }

    template <class... Types>
    auto multiRegisterLN(std::string_view file, unsigned int line,
                         std::string_view level, std::string_view format,
                         Types &&...args) -> std::string {
        std::string printbuf = Strutils::multi_concat(
            file, ":", std::to_string(line), " ", level, " ",
            StrFormat::multiRegister(format, std::forward<Types>(args)...));

        AddToLog(printbuf);
        return printbuf;
    }

    void PrepareToFork();
    void SignalFork();
    void ParentPostFork();

    void FinishLog();
    void operator<<(const std::string &Text) { AddToLog(Text); }

    static auto log() -> CLog &;
    static auto initSingleton(const logOutputInfo &logcfg = {}) -> CLog &;
    static auto initSingleton(std::string_view filepath) -> CLog &;

    CLog(const CLog &) = delete;
    CLog(CLog &&) = delete;

    auto operator=(const CLog &) -> CLog & = delete;
    auto operator=(CLog &&) -> CLog & = delete;

    static logOutputInfo defaultcfg;

    explicit CLog(const logOutputInfo &logcfg = {});

  private:
    static auto addLinesToLog(CLog &logInst) -> bool;
    static void threadFn(CLog &logInst);

    std::fstream LogFile;
    std::thread writterThreadInst;
    std::mutex cmdmtx;
    std::condition_variable waitcmd;
    std::atomic<bool> running;

    std::unique_ptr<ScopedStreamRedirect> streamRedirect;
    bool Finished;

    void insertLogHeader();
};
#endif
