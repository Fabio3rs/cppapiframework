#include "projstdafx.hpp"

#include <gtest/gtest.h>

// NOLINTNEXTLINE
TEST(StrFormatTest, MultiRegisterStrEqual) {
    EXPECT_EQ(StrFormat::multiRegister(StrFormat::LineBreakPolicy::Keep,
                                       "Teste log (%0) AAAAAAAAAAAA", 10),
              "Teste log (10) AAAAAAAAAAAA");
    EXPECT_EQ(StrFormat::multiRegister(StrFormat::LineBreakPolicy::Keep,
                                       "Teste log (%0) AAAAAAAAAAAA %1 | %2",
                                       10, "Teste", 20),
              "Teste log (10) AAAAAAAAAAAA Teste | 20");
    EXPECT_EQ(StrFormat::multiRegister(StrFormat::LineBreakPolicy::Keep,
                                       "Teste log (%0) AAAAAAAAAAAA %1", 10),
              "Teste log (10) AAAAAAAAAAAA %1");
}

TEST(StrFormatTest, LineBreakPolicies) {
    using StrFormat::LineBreakPolicy;
    for (auto policy : {LineBreakPolicy::Remove, LineBreakPolicy::Escape,
                        LineBreakPolicy::Keep}) {
        const std::string expected = policy == LineBreakPolicy::Remove ? "abc"
                                     : policy == LineBreakPolicy::Escape
                                         ? "a\\r\\nb\\nc\\r"
                                         : "a\r\nb\nc\r";
        EXPECT_EQ(StrFormat::multiRegister(policy, "a\r\nb\nc\r"), expected);
        EXPECT_EQ(StrFormat::multiRegister(policy, "%0", "a\r\nb\nc\r"),
                  expected);
        EXPECT_EQ(StrFormat::multiRegister(policy, "a\r\n%0c\r", "b\n"),
                  expected);
        EXPECT_EQ(StrFormat::multiRegister(policy, "%0%0", "a\r\nb\nc\r"),
                  expected + expected);
        EXPECT_EQ(StrFormat::multiRegister(policy, ""), "");
        EXPECT_EQ(StrFormat::multiRegister(policy, "%0", ""), "");
    }
}

TEST(StrFormatTest, LineBreakDoesNotEscapeNextPlaceholder) {
    using StrFormat::LineBreakPolicy;
    EXPECT_EQ(StrFormat::multiRegister(LineBreakPolicy::Remove, "\\\n%0", "ok"),
              "ok");
    EXPECT_EQ(StrFormat::multiRegister(LineBreakPolicy::Escape, "\\\n%0", "ok"),
              "\\nok");
    EXPECT_EQ(StrFormat::multiRegister(LineBreakPolicy::Keep, "\\\n%0", "ok"),
              "\nok");
}

TEST(StrFormatTest, PreserveExistingEscapesAndArgumentText) {
    for (auto policy : {StrFormat::LineBreakPolicy::Remove,
                        StrFormat::LineBreakPolicy::Escape,
                        StrFormat::LineBreakPolicy::Keep}) {
        EXPECT_EQ(
            StrFormat::multiRegister(policy, R"(\%0 \\ %0 %2)", R"(\n\r%1)"),
            R"(%0 \ \n\r%1 %2)");
    }
}

TEST(StrFormatTest, ExceptionAndJsonArguments) {
    using StrFormat::LineBreakPolicy;
    const std::runtime_error error("first\nsecond\r");
    EXPECT_EQ(StrFormat::multiRegister(LineBreakPolicy::Escape, "%0", error),
              "first\\nsecond\\r");
    Poco::JSON::Array::Ptr array = new Poco::JSON::Array;
    array->add("first\nsecond");
    EXPECT_EQ(StrFormat::multiRegister(LineBreakPolicy::Escape, "%0", array),
              "[\"first\\nsecond\"]");
    Poco::JSON::Array::Ptr nullArray;
    EXPECT_EQ(
        StrFormat::multiRegister(LineBreakPolicy::Escape, "%0", nullArray),
        "{NULL JSON}");
}
