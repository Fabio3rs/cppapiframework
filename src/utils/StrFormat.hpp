#pragma once

#include "PocoJsonStringify.hpp"
#include <Poco/JSON/Object.h>
#include <array>
#include <string>
#include <tuple>

namespace StrFormat {
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

    template <class T,
              typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(T value) : str(std::to_string(value)) {}
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
inline auto multiRegister(std::string_view format, Types &&...args)
    -> std::string {
    const std::array<argToString, std::tuple_size<std::tuple<Types...>>::value>
        // NOLINTNEXTLINE(hicpp-no-array-decay)
        argl = {std::forward<argToString>(args)...};
    std::string printbuf;
    printbuf.reserve(format.size() + argl.size() * 8);

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
                        printbuf += argl[argId].getStr();
                    } else {
                        printbuf += "%";
                        printbuf += numbuf;
                    }
                }
            }
            break;

        default:
            ignoreNext = false;
            printbuf.insert(printbuf.end(), 1, curCh);
            break;
        }
    }

    return printbuf;
}
} // namespace StrFormat
