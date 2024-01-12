#include "projstdafx.hpp"

#include <gtest/gtest.h>

// NOLINTNEXTLINE
TEST(StrFormatTest, MultiRegisterStrEqual) {
    EXPECT_EQ(StrFormat::multiRegister("Teste log (%0) AAAAAAAAAAAA", 10),
              "Teste log (10) AAAAAAAAAAAA");
    EXPECT_EQ(StrFormat::multiRegister("Teste log (%0) AAAAAAAAAAAA %1 | %2", 10,
                                "Teste", 20),
              "Teste log (10) AAAAAAAAAAAA Teste | 20");
    EXPECT_EQ(StrFormat::multiRegister("Teste log (%0) AAAAAAAAAAAA %1", 10),
              "Teste log (10) AAAAAAAAAAAA %1");
}

