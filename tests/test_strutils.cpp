#include "../src/utils/Strutils.hpp"
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
