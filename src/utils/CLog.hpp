#pragma once
#ifndef LOGGING_SYSTEM_CLOG_H
#define LOGGING_SYSTEM_CLOG_H
#include <atomic>
#include <condition_variable>
#include <exception>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class CLog {
    std::fstream LogFile;
    std::thread writterThreadInst;
    std::mutex cmdmtx;
    std::condition_variable waitcmd;
    std::atomic<bool> running;

    static auto addLinesToLog(CLog &logInst) -> bool;
    static void threadFn(CLog &logInst);

    class argToString {
        const std::string str;

      public:
        [[nodiscard]] auto getStr() const -> const std::string & { return str; }

        argToString(bool value) : str(value ? "true" : "false") {}

        argToString(const char *s) : str(s) {}

        argToString(const std::exception &e) : str(e.what()) {}

        argToString(std::string s) : str(std::move(s)) {}

        template <class T,
                  typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        argToString(T value) : str(std::to_string(value)) {}
    };

    bool Finished;

  public:
    ~CLog() noexcept;
    void AddToLog(const std::string &Text, const std::string &extraid = "");

    template <class... Types>
    auto multiRegister(const std::string &format, Types &&... args)
        -> std::string {
        const std::array<argToString,
                         std::tuple_size<std::tuple<Types...>>::value>
            a = {std::forward<Types>(args)...};
        std::string printbuf;

        std::string numbuf;

        bool ignoreNext = false;

        for (size_t i = 0, size = format.size(); i < size; i++) {
            auto ch = format[i];
            size_t ti = i + 1;

            switch (ch) {
            case '\\':
                if (ignoreNext) {
                    printbuf.insert(printbuf.end(), 1, ch);
                    ignoreNext = false;
                    break;
                }

                ignoreNext = true;
                break;

            case '%':
                if (ignoreNext) {
                    printbuf.insert(printbuf.end(), 1, ch);
                    ignoreNext = false;
                    break;
                }

                numbuf = "";

                {
                    bool stringEnd = true;
                    while (ti < size) {
                        if (format[ti] < '0' || format[ti] > '9') {
                            i = ti - 1;
                            if (!numbuf.empty()) {
                                size_t argId = std::stoul(numbuf);

                                if (argId < a.size()) {
                                    printbuf += a[argId].getStr();
                                } else {
                                    printbuf += "%";
                                    printbuf += numbuf;
                                }

                                stringEnd = false;

                                break;
                            }
                        } else {
                            numbuf.insert(numbuf.end(), 1, format[ti]);
                        }

                        ti++;
                    }

                    if (stringEnd) {
                        i = size;
                        if (!numbuf.empty()) {
                            size_t argId = std::stoul(numbuf);

                            if (argId < a.size()) {
                                printbuf += a[argId].getStr();
                            } else {
                                printbuf += "%";
                                printbuf += numbuf;
                            }
                        }
                    }
                }
                break;

            default:
                ignoreNext = false;
                printbuf.insert(printbuf.end(), 1, ch);
                break;
            }
        }

        AddToLog(printbuf);
        return printbuf;
    }

    void FinishLog();
    void operator<<(const std::string &Text) { AddToLog(Text); }

    static auto log(const char *filepath = nullptr) -> CLog &;

    CLog(const CLog &) = delete;

  private:
    explicit CLog(const std::string &NameOfFile);
};
#endif
