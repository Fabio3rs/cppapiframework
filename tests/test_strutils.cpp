#include "projstdafx.hpp"

#include <gtest/gtest.h>

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
TEST(TestStrutils, JoinTest) {
    std::vector<std::string> vec1 = {"hello", "world", "!"};
    std::string result1 = Strutils::join(vec1, " ");
    EXPECT_EQ(result1, "hello world !");

    std::vector<std::string> vec2 = {"apple", "banana", "cherry"};
    std::string result2 = Strutils::join(vec2, ", ");
    EXPECT_EQ(result2, "apple, banana, cherry");

    std::vector<std::string> vec3 = {"1", "2", "3", "4", "5"};
    std::string result3 = Strutils::join(vec3, "");
    EXPECT_EQ(result3, "12345");

    std::vector<std::string> vec4 = {"", "", ""};
    std::string result4 = Strutils::join(vec4, " ");
    EXPECT_EQ(result4, "  ");

    std::vector<std::string> vec5 = {};
    std::string result5 = Strutils::join(vec5, " ");
    EXPECT_EQ(result5, "");

    const std::vector<std::string> cvec2 = {"apple", "banana", "cherry"};
    std::string cresult2 = Strutils::join(vec2, ", ");
    EXPECT_EQ(cresult2, "apple, banana, cherry");
}
