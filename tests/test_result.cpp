#include "../src/utils/ResultMacros.hpp"
#include <atomic>
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

// NOLINTNEXTLINE
TEST(TestResult, SingleCallTest) {
    std::atomic<int> callCount = 0;
    using res_t = Result<okvalue, std::string>;
    auto someFn = [&]() -> res_t {
        ++callCount;
        return res_t::ok_t{{callCount.load()}};
    };

    auto callWrapper = [&]() -> res_t {
        return res_t::ok_t{EXPECT_RESULT(someFn())};
    };

    EXPECT_EQ(callWrapper().unwrap().val, 1);
}

// NOLINTNEXTLINE
TEST(TestResult, SingleCallTestErr) {
    std::atomic<int> callCount = 0;
    using res_t = Result<okvalue, std::string>;
    auto someFn = [&]() -> res_t {
        ++callCount;
        return res_t::error_t{"#"};
    };

    auto callWrapper = [&]() -> res_t {
        return res_t::ok_t{EXPECT_RESULT(someFn())};
    };

    EXPECT_EQ(callWrapper().unwrap_error().get(), "#");
    EXPECT_EQ(callCount.load(), 1);
}

// NOLINTNEXTLINE
TEST(TestResult, SingleCallTestE) {
    std::atomic<int> callCount = 0;
    using res_t = Result<okvalue, std::string>;
    auto someFn = [&]() -> res_t {
        ++callCount;
        return res_t::ok_t{{callCount.load()}};
    };

    auto callWrapper = [&]() -> res_t {
        return res_t::ok_t{EXPECT_RESULT_E(someFn(), res_t::error_t{""})};
    };

    EXPECT_EQ(callWrapper().unwrap().val, 1);
}
