#pragma once

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
    argToString(const char *s) : str(s) {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(const std::exception &e) : str(e.what()) {}

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(std::string s) : str(std::move(s)) {}

    template <class T,
              typename = std::enable_if_t<std::is_arithmetic<T>::value>>
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    argToString(T value) : str(std::to_string(value)) {}
};

template <class... Types>
inline auto multiRegister(std::string_view format, Types &&... args)
    -> std::string {
    const std::array<argToString, std::tuple_size<std::tuple<Types...>>::value>
        a = {std::forward<Types>(args)...};
    std::string printbuf;
    printbuf.reserve(format.size() + a.size() * 8);

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

    return printbuf;
}
} // namespace StrFormat
