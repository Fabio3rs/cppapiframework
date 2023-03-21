#include <gtest/gtest.h>
#include "../src/utils/StrFormat.hpp"
#include "../src/Mail/Mail.hpp"

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StrFormatTest, MultiRegisterStrEqual) {
    EXPECT_EQ(StrFormat::multiRegister("Teste log (%0) AAAAAAAAAAAA", 10),
              "Teste log (10) AAAAAAAAAAAA");
    EXPECT_EQ(StrFormat::multiRegister("Teste log (%0) AAAAAAAAAAAA %1 | %2", 10,
                                "Teste", 20),
              "Teste log (10) AAAAAAAAAAAA Teste | 20");
    EXPECT_EQ(StrFormat::multiRegister("Teste log (%0) AAAAAAAAAAAA %1", 10),
              "Teste log (10) AAAAAAAAAAAA %1");
}


// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(StrFormatTest, aaaaaaaa) {
    mailbase::Mail mail;

    mail.build();

    //EXPECT_TRUE(false);
}


