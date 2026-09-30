#pragma once

#include "PocoJsonStringify.hpp"
#include <Poco/JSON/Object.h>
#include <array>
#include <cstdint>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace StrFormat {

// Applies to actual CR/LF characters in both the format and its arguments.
// Escape emits literal \\r/\n; already escaped text is not escaped again.
enum class LineBreakPolicy : std::uint8_t { Remove, Escape, Keep };

class argToString {
    std::string str;

  public:
    [[nodiscard]] auto getStr() const -> const std::string & { return str; }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(bool value) : str(value ? "true" : "false") {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(const char *istr) : str(istr) {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(const std::exception &excp) : str(excp.what()) {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(std::string istr) : str(std::move(istr)) {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(const Poco::JSON::Object::Ptr &jsonobj) {
        if (jsonobj.isNull()) {
            str = "{NULL JSON}";
        } else {
            PocoJsonStringify stringifier;
            stringifier.stringify(jsonobj);

            str = std::move(stringifier.str);
        }
    }

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(const Poco::JSON::Array::Ptr &jsonarr) {
        if (jsonarr.isNull()) {
            str = "{NULL JSON}";
        } else {
            PocoJsonStringify stringifier;
            stringifier.stringify(jsonarr);

            str = std::move(stringifier.str);
        }
    }

    template <class T>
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(T value)
        requires std::is_arithmetic_v<T>
        : str(std::to_string(value)) {}
};

inline auto getNumericFromString(std::string_view str) -> std::string {
    std::string numbuf;
    for (const auto &chr : str) {
        if (chr < '0' || chr > '9') {
            break;
        }

        numbuf.insert(numbuf.end(), 1, chr);
    }

    return numbuf;
}

template <class... Types>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
inline auto multiRegister(LineBreakPolicy lbPolicy, std::string_view format,
                          Types &&...args) -> std::string {
    const std::array<argToString, sizeof...(Types)> argl = {
        argToString(std::forward<Types>(args))...};
    std::string printbuf;
    printbuf.reserve(format.size() +
                     std::accumulate(argl.begin(), argl.end(), size_t{0},
                                     [](size_t sum, const argToString &arg) {
                                         return sum + arg.getStr().size();
                                     }));

    const auto append = [&](std::string_view text) {
        if (lbPolicy == LineBreakPolicy::Keep) {
            printbuf += text;
            return;
        }
        size_t start = 0;
        for (size_t pos = 0; pos < text.size(); ++pos) {
            if (text[pos] != '\r' && text[pos] != '\n') {
                continue;
            }
            printbuf += text.substr(start, pos - start);
            if (lbPolicy == LineBreakPolicy::Escape) {
                printbuf += text[pos] == '\r' ? "\\r" : "\\n";
            }
            start = pos + 1;
        }
        printbuf += text.substr(start);
    };

    bool ignoreNext = false;

    for (size_t i = 0, size = format.size(); i < size; i++) {
        auto curCh = format[i];

        switch (curCh) {
        case '\\':
            if (ignoreNext) {
                printbuf.insert(printbuf.end(), 1, curCh);
                ignoreNext = false;
                break;
            }

            ignoreNext = true;
            break;

        case '%':
            if (ignoreNext) {
                printbuf.insert(printbuf.end(), 1, curCh);
                ignoreNext = false;
                break;
            }

            {
                std::string numbuf = getNumericFromString(format.substr(i + 1));

                i += numbuf.size();

                if (!numbuf.empty()) {
                    size_t argId = std::stoul(numbuf);

                    if (argId < argl.size()) {
                        append(argl[argId].getStr());
                    } else {
                        printbuf += "%";
                        printbuf += numbuf;
                    }
                }
            }
            break;

        default:
            ignoreNext = false;
            append(format.substr(i, 1));
            break;
        }
    }

    return printbuf;
}
} // namespace StrFormat
