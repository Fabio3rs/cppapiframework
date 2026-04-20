#include "utils/Strutils.hpp"
#include <gtest/gtest.h>

TEST(StrutilsCNPJ, RemovesPunctuationAndKeepsDigits) {
    const std::string input = "12.345.678/0001-95";
    const std::string out = Strutils::getCNPJNumbers(input);
    EXPECT_EQ(out, "12345678000195");
}

TEST(StrutilsCNPJ, KeepsLettersAndDigitsForFutureFormat) {
    const std::string input = "AB12.CD34/EF56-78";
    const std::string out = Strutils::getCNPJNumbers(input);
    EXPECT_EQ(out, "AB12CD34EF5678");
}

TEST(StrutilsCNPJ, RemovesSpacesAndSymbols) {
    const std::string input = " A B . 1 2 - 3 4 ";
    const std::string out = Strutils::getCNPJNumbers(input);
    EXPECT_EQ(out, "AB1234");
}

TEST(StrutilsCNPJ, EmptyStringReturnsEmpty) {
    const std::string input;
    const std::string out = Strutils::getCNPJNumbers(input);
    EXPECT_EQ(out, "");
}
