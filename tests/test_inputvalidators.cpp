/**
 *@file test_inputvalidators.cpp
 * @author Fabio Rossini Sluzala ()
 * @brief teste do validador de email
 * @version 0.1
 * @date 2021-07-26
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../src/utils/ControllerInputValidator.hpp"
#include <gtest/gtest.h>

const std::string_view mockfname = "teste";

/**
 *@brief Checagem com diversos tipos de email
 *@todo Usar mais tipos de emails
 *
 */
TEST(InputValidatorTest, CheckEmail) {
    EmailValidator emailval;

    EXPECT_FALSE(emailval.validate(mockfname, "").isEmpty());
    EXPECT_FALSE(emailval.validate(mockfname, "aaaaa").isEmpty());
    EXPECT_FALSE(emailval.validate(mockfname, "aaaaa@bbbb").isEmpty());
    EXPECT_TRUE(emailval.validate(mockfname, "aaaaa@bbbb.com").isEmpty());
    EXPECT_TRUE(emailval.validate(mockfname, "aaa.aa@bbbb.com").isEmpty());
    EXPECT_TRUE(emailval.validate(mockfname, "aaa_aa@bbbb.com").isEmpty());
}

TEST(InputValidatorTest, CheckIntegerValidator) {
    IntegerValidator intval;

    EXPECT_FALSE(intval.validate(mockfname, "").isEmpty());
    EXPECT_FALSE(intval.validate(mockfname, "aaaaa").isEmpty());
    EXPECT_FALSE(intval.validate(mockfname, "a-123456").isEmpty());
    EXPECT_TRUE(intval.validate(mockfname, "123456").isEmpty());
    EXPECT_TRUE(intval.validate(mockfname, "123456789101112").isEmpty());
    EXPECT_TRUE(intval.validate(mockfname, "123456789101112").isEmpty());
}

TEST(InputValidatorTest, CheckRequiredValidator) {
    RequiredValidator reqval;

    EXPECT_FALSE(reqval.validate(mockfname, {}).isEmpty());
    EXPECT_TRUE(reqval.validate(mockfname, "").isEmpty());
    EXPECT_TRUE(reqval.validate(mockfname, "aaaaa").isEmpty());
    EXPECT_TRUE(reqval.validate(mockfname, "a-123456").isEmpty());
    EXPECT_TRUE(reqval.validate(mockfname, "123456").isEmpty());
    EXPECT_TRUE(reqval.validate(mockfname, "123456789101112").isEmpty());
    EXPECT_TRUE(reqval.validate(mockfname, "123456789101112").isEmpty());
    EXPECT_TRUE(reqval.validate(mockfname, "aaa_aa@bbbb.com").isEmpty());
}

TEST(InputValidatorTest, CheckObjectValidator) {
    ObjectValidator objval(
        [](std::string_view, ControllerInputValidator &validator) {
            validator.full_validation(
                std::pair("testeval", std::tuple(RequiredValidator())));
            return validator.get_only_messages();
        });

    EXPECT_TRUE(objval.validate(mockfname, {}).isEmpty());
    EXPECT_FALSE(objval.validate(mockfname, "").isEmpty());
    EXPECT_FALSE(objval.validate(mockfname, "aaaaa").isEmpty());
    EXPECT_FALSE(objval.validate(mockfname, "a-123456").isEmpty());
    EXPECT_FALSE(objval.validate(mockfname, "123456").isEmpty());
    EXPECT_FALSE(objval.validate(mockfname, "123456789101112").isEmpty());
    EXPECT_FALSE(objval.validate(mockfname, "123456789101112").isEmpty());
    EXPECT_FALSE(objval.validate(mockfname, "aaa_aa@bbbb.com").isEmpty());

    Poco::JSON::Object::Ptr mockobj(new Poco::JSON::Object);

    EXPECT_FALSE(objval.validate(mockfname, mockobj).isEmpty());

    mockobj->set("testeval", "strtest");
    EXPECT_TRUE(objval.validate(mockfname, mockobj).isEmpty());
}

TEST(InputValidatorTest, CheckArrayValidator) {
    ArrayValidator arrval(
        [](std::string_view, size_t pos, Poco::Dynamic::Var value) {
            if (value != "teste") {
                return Poco::Dynamic::Var("Incorrect value at " +
                                          std::to_string(pos));
            }
            return Poco::Dynamic::Var{};
        });

    EXPECT_TRUE(arrval.validate(mockfname, {}).isEmpty());
    EXPECT_FALSE(arrval.validate(mockfname, "").isEmpty());
    EXPECT_FALSE(arrval.validate(mockfname, "aaaaa").isEmpty());
    EXPECT_FALSE(arrval.validate(mockfname, "a-123456").isEmpty());
    EXPECT_FALSE(arrval.validate(mockfname, "123456").isEmpty());
    EXPECT_FALSE(arrval.validate(mockfname, "123456789101112").isEmpty());
    EXPECT_FALSE(arrval.validate(mockfname, "123456789101112").isEmpty());
    EXPECT_FALSE(arrval.validate(mockfname, "aaa_aa@bbbb.com").isEmpty());

    {
        Poco::JSON::Object::Ptr mockobj(new Poco::JSON::Object);

        EXPECT_FALSE(arrval.validate(mockfname, mockobj).isEmpty());
    }

    Poco::JSON::Array::Ptr mockarr(new Poco::JSON::Array);
    EXPECT_TRUE(arrval.validate(mockfname, mockarr).isEmpty());

    mockarr->add("teste");

    EXPECT_TRUE(arrval.validate(mockfname, mockarr).isEmpty());

    mockarr->add("fail test");

    EXPECT_FALSE(arrval.validate(mockfname, mockarr).isEmpty());
}

TEST(InputValidatorTest, CheckOrValidator) {
    auto orval = make_orvalidator(IntegerValidator(), EmailValidator());

    EXPECT_FALSE(orval.validate(mockfname, "").isEmpty());
    EXPECT_FALSE(orval.validate(mockfname, "aaaaa").isEmpty());
    EXPECT_FALSE(orval.validate(mockfname, "a-123456").isEmpty());

    EXPECT_FALSE(orval.validate(mockfname, "aaaaa@bbbb").isEmpty());
    EXPECT_TRUE(orval.validate(mockfname, "aaaaa@bbbb.com").isEmpty());
    EXPECT_TRUE(orval.validate(mockfname, "aaa.aa@bbbb.com").isEmpty());
    EXPECT_TRUE(orval.validate(mockfname, "aaa_aa@bbbb.com").isEmpty());
    EXPECT_TRUE(orval.validate(mockfname, "123456").isEmpty());
    EXPECT_TRUE(orval.validate(mockfname, "123456789101112").isEmpty());
    EXPECT_TRUE(orval.validate(mockfname, "123456789101112").isEmpty());
}
