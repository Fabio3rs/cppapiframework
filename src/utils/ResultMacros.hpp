#pragma once

#include "Result.hpp"

#define CHECKRESERR(type)                                                      \
    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, utils::Err<type>>)
#define CHECKRESOK(type)                                                       \
    if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, utils::Ok<type>>)
#define MATCHRESULT(result, body)                                              \
    { std::visit([&](auto &&arg) body, result); }

#define EXPECT_RESULT_E(test, error)                                           \
    ({                                                                         \
        auto value = test;                                                     \
        if (value.state() != utils::State::OkState) {                          \
            return error;                                                      \
        }                                                                      \
        value.unwrap();                                                        \
    })

#define EXPECT_RESULT(test)                                                    \
    ({                                                                         \
        auto value = test;                                                     \
        if (value.state() != utils::State::OkState) {                          \
            return std::move(value.unwrap_error());                            \
        }                                                                      \
        value.unwrap();                                                        \
    })
