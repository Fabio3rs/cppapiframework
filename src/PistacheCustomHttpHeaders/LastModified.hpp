#pragma once

#include "../stdafx.hpp"
#include <bits/types/time_t.h>
#include <memory>
#include <string>

namespace PistacheCustomHttpHeaders {
class LastModified : public Pistache::Http::Header::Header {
  public:
    // NOLINTNEXTLINE
    NAME("Last-Modified")

    void parseRaw(const char *str, size_t len) override {
        formatedDate.append(str, len);
    }

    // NOLINTNEXTLINE
    void write(std::ostream &os) const override {
        if (formatedDate.empty()) {
            return;
        }

        os << formatedDate;
    }

    static auto getTimeFromStr(const std::string &dateTime) -> time_t {
        std::tm tmHeader{};
        std::stringstream ssdate(dateTime);
        ssdate >> std::get_time(&tmHeader, "%a, %d %b %Y %H:%M:%S %Z");

        return std::mktime(&tmHeader);
    }

    [[nodiscard]] auto getTimeT() const -> std::time_t {
        return getTimeFromStr(formatedDate);
    }

    static auto format(std::time_t cftime) -> std::string {
        std::array<char, 48> buffer{};

        size_t strft_res_sz =
            strftime(buffer.data(), buffer.size(), "%a, %d %b %Y %H:%M:%S GMT",
                     std::gmtime(&cftime));

        return {buffer.data(), strft_res_sz};
    }

    template <class T>
    static auto timeCast(std::chrono::time_point<T> timePoint) -> time_t {
        using namespace std::chrono;
        auto sctp = time_point_cast<system_clock::duration>(
            timePoint - std::chrono::time_point<T>::clock::now() +
            system_clock::now());
        return system_clock::to_time_t(sctp);
    }

    template <class T>
    static auto format(std::chrono::time_point<T> timePoint) -> std::string {
        return format(timeCast(timePoint));
    }

    static auto make(std::time_t cftime) {
        return std::make_shared<LastModified>(cftime);
    }

    template <class T> static auto make(std::chrono::time_point<T> timePoint) {
        return std::make_shared<LastModified>(timePoint);
    }

    LastModified() = default;
    LastModified(const LastModified &) = default;
    LastModified(LastModified &&) = delete;
    auto operator=(const LastModified &) -> LastModified & = default;
    auto operator=(LastModified &&) -> LastModified & = delete;

    explicit LastModified(std::string Date) : formatedDate(std::move(Date)) {}
    explicit LastModified(std::time_t cftime) : formatedDate(format(cftime)) {}

    template <class T>
    explicit LastModified(std::chrono::time_point<T> timePoint)
        : formatedDate(format(timePoint)) {}

    ~LastModified() override;

    std::string formatedDate;
};
} // namespace PistacheCustomHttpHeaders
