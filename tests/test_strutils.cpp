#include "utils/Strutils.hpp"

#include <array>
#include <forward_list>
#include <gtest/gtest.h>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestStrutils, TestMultiConcat) {
    EXPECT_EQ(Strutils::multi_concat("a", std::to_string(123), "b", "c"),
              "a123bc");
    EXPECT_EQ(
        Strutils::multi_concat(std::to_string(1000000), "qwertyui", "abc", "d"),
        "1000000qwertyuiabcd");

    std::string_view tmp = "12345678910123456789";
    EXPECT_EQ(Strutils::multi_concat(tmp, "abc", "d", "123456789",
                                     std::to_string(987654321), "a"),
              "12345678910123456789abcd123456789987654321a");
}

// NOLINTNEXTLINE
TEST(TestStrutils, TestExplodeFunction) {
    std::vector<std::string> fullvecabc{"a", "b", "c"};
    std::vector<std::string> fullvecab{"a", "b"};
    EXPECT_EQ(Strutils::explode("a,b,c", ","), fullvecabc);
    EXPECT_EQ(Strutils::explode("a,b", ","), fullvecab);
    EXPECT_EQ(Strutils::explode("", ","), std::vector<std::string>{});
    EXPECT_EQ(Strutils::explode("", ""), std::vector<std::string>{});
    EXPECT_EQ(Strutils::explode("a,b,c", ""),
              std::vector<std::string>{"a,b,c"});
    EXPECT_EQ(Strutils::explode("a,b,c", ","), fullvecabc);
}

// NOLINTNEXTLINE
TEST(TestStrutils, JoinHandlesSeparatorsAndEmptyValues) {
    const std::vector<std::string> words = {"hello", "world", "!"};
    EXPECT_EQ(Strutils::join(words, " "), "hello world !");
    EXPECT_EQ(Strutils::join(words, ""), "helloworld!");

    const std::vector<std::string> empty_values = {"", "", ""};
    EXPECT_EQ(Strutils::join(empty_values, "--"), "----");

    const std::vector<std::string> one_value = {"only"};
    EXPECT_EQ(Strutils::join(one_value, "ignored"), "only");

    const std::vector<std::string> no_values;
    EXPECT_TRUE(Strutils::join(no_values, " ").empty());
}

// NOLINTNEXTLINE
TEST(TestStrutils, JoinSupportsSpanOverload) {
    const std::array<std::string, 3> values = {"one", "two", "three"};
    const std::span<const std::string> values_span{values};

    EXPECT_EQ(Strutils::join(values_span, "::"), "one::two::three");
}

// NOLINTNEXTLINE
TEST(TestStrutils, JoinSupportsUnsizedRangesAndStringViews) {
    const std::forward_list<std::string> values = {"one", "two", "three"};
    EXPECT_EQ(Strutils::join(values, "/"), "one/two/three");

    constexpr std::array<std::string_view, 3> views = {"alpha", "beta",
                                                       "gamma"};
    EXPECT_EQ(Strutils::join(views, " | "), "alpha | beta | gamma");
}
