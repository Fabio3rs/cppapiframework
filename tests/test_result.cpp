#include "../src/utils/ResultMacros.hpp"
#include <gtest/gtest.h>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace {
using utils::Err;
using utils::Ok;
using utils::Result;

struct okvalue {
    int val{};
};

auto failToEvaluateSomething() -> Result<okvalue, std::string> {
    return Err<std::string>{"Some error description"};
}

auto okToEvaluateSomething() -> Result<okvalue, std::string> {
    return Ok<okvalue>{};
}

auto earlyReturnEvaluateSomething() -> Result<okvalue, std::string> {
    auto okVal = EXPECT_RESULT(failToEvaluateSomething());

    EXPECT_FALSE(true);

    return Ok<decltype(okVal)>{okVal};
}

auto notEarlyReturnEvaluateSomething() -> Result<okvalue, std::string> {
    auto okVal = EXPECT_RESULT(okToEvaluateSomething());

    return Ok<decltype(okVal)>{{okVal.val + 1}};
}
} // namespace

// NOLINTNEXTLINE
TEST(TestResult, TestBase) {
    std::visit(
        [](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, utils::Ok<okvalue>>) {
                EXPECT_TRUE(false);
            } else if constexpr (std::is_same_v<T, utils::Err<std::string>>) {
                EXPECT_EQ(arg.get(), "Some error description");
            }
        },
        failToEvaluateSomething().variant());
}

// NOLINTNEXTLINE
TEST(TestResult, MatchFail) {
    bool path = false;
    MATCHRESULT(failToEvaluateSomething().variant(), {
        CHECKRESOK(okvalue) { EXPECT_TRUE(false); };

        CHECKRESERR(std::string) { path = true; };
    })

    EXPECT_TRUE(path);
}

// NOLINTNEXTLINE
TEST(TestResult, MatchOk) {
    bool path = false;
    MATCHRESULT(okToEvaluateSomething().variant(), {
        CHECKRESOK(okvalue) { path = true; };

        CHECKRESERR(std::string) { EXPECT_TRUE(false); };
    })

    EXPECT_TRUE(path);
}

// NOLINTNEXTLINE
TEST(TestResult, EarlyReturn) {
    EXPECT_EQ(earlyReturnEvaluateSomething().unwrap_error().get(),
              "Some error description");
}

// NOLINTNEXTLINE
TEST(TestResult, NotEarlyReturn) {
    EXPECT_EQ(notEarlyReturnEvaluateSomething().unwrap().val, 1);
}
