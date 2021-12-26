#pragma once
#ifndef Valdiator_hpp
#define Valdiator_hpp

#include "../stdafx.hpp"
#include <array>
#include <cctype>
#include <limits>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <string_view>
#include <tuple>
#include <utility>

class ValidatorException : public std::exception {
    std::string msg, fieldname;

  public:
    [[nodiscard]] auto what() const noexcept -> const char * override;

    inline ValidatorException(std::string w, std::string fname) noexcept
        : msg(std::move(w)), fieldname(std::move(fname)) {}

    [[nodiscard]] auto to_json() const -> Poco::JSON::Object::Ptr;

    ~ValidatorException() override = default;
};

class Validator {
  public:
    static inline auto parse_json_from_string(const std::string &str)
        -> Poco::JSON::Object::Ptr {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var jsonvar = parser.parse(str);
        return jsonvar.extract<Poco::JSON::Object::Ptr>();
    }

    static inline auto
    request_to_json(const Pistache::Rest::Request &request, size_t minsize = 0,
                    size_t maxsize = std::numeric_limits<size_t>::max())
        -> Poco::JSON::Object::Ptr {
        if (request.body().size() < minsize) {
            throw ValidatorException("Input data is small than expected",
                                     "input");
        }

        if (request.body().size() > maxsize) {
            throw ValidatorException("Input data is larger than expected",
                                     "input");
        }

        return parse_json_from_string(request.body());
    }

    static inline auto is_alphanum(int ch) -> bool { return isalnum(ch) != 0; }

    template <class T, class... Types>
    static inline constexpr auto custom_array(Types... args) {
        std::array<T, std::tuple_size<std::tuple<Types...>>::value> a = {
            std::forward<Types>(args)...};

        return a;
    }

    static auto CheckSQL(const std::string &sql) -> bool {
        auto key = custom_array<std::string_view>("%", "/", "union", "|", "&",
                                                  "^", "#", "/*", "*/");

        for (const auto &str : key) {
            if (sql.find(str) != std::string::npos) {
                return false;
            }
        }
        return true;
    }

    static auto CheckParameter(const std::string &Parameter) -> bool {
        auto key = custom_array<std::string_view>("and", "*", "=", " ", "%0a",
                                                  "%", "/", "union", "|", "&",
                                                  "^", "#", "/*", "*/");
        for (const auto &str : key) {
            if (Parameter.find(str) != std::string::npos) {
                return false;
            }
        }
        return true;
    }
};

#endif
