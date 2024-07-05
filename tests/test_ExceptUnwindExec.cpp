#include "projstdafx.hpp"

#include <gtest/gtest.h>

TEST(ExceptUnwindTest, ExceptUnwindExec) {
    int calledTimes = 0;
    auto callbackfn = [&]() -> void { ++calledTimes; };

    {
        ExceptUnwindExec exec(callbackfn);
        EXPECT_EQ(calledTimes, 0);
    }

    EXPECT_EQ(calledTimes, 0);
}

[[noreturn]] static void someErrorFun(int &counter) {
    ExceptUnwindExec exec([&counter]() -> void {
        ++counter;
        std::cerr << "someErrorFun" << std::endl;
    });

    throw std::runtime_error("Test");
}

TEST(ExceptUnwindTest, ExceptUnwindExecInFunc) {
    int counter = 0;
    try {
        someErrorFun(counter);
    } catch (const std::exception &e) {
        std::cerr << __FILE__ << "   " << e.what() << std::endl;
    }

    EXPECT_EQ(counter, 1);
}

TEST(ExceptUnwindTest, ExceptUnwindExecThrow) {
    int calledTimes = 0;
    auto callbackfn = [&]() -> void { ++calledTimes; };

    try {
        ExceptUnwindExec exec(callbackfn);
        EXPECT_EQ(calledTimes, 0);

        throw std::runtime_error("Test");
    } catch (const std::exception &e) {
        std::cerr << __FILE__ << "   " << e.what() << std::endl;
    }

    EXPECT_EQ(calledTimes, 1);
}
