#include "projstdafx.hpp"

#include "allocation_count.hpp"
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <string_view>

// NOLINTNEXTLINE
TEST(TestStringstream, Stream) {
    AllocationCount::getAllocationCount() = 0;
    size_t start = AllocationCount::getAllocationCount().load();

    std::array<int, 9> correctNums{123456, 123456, 789, 10, 11, 12, 13, 14, 15};
    stringstream_view strv("123456 123456 789 10 11 12 13 14 15");

    ASSERT_EQ(AllocationCount::getAllocationCount().load(), start);

    std::istream istr(&strv);

    ASSERT_EQ(AllocationCount::getAllocationCount().load(), start);

    std::array<int, 9> nums{};
    for (int &num : nums) {
        istr >> num;
    }

    ASSERT_EQ(AllocationCount::getAllocationCount().load(), start);
    ASSERT_EQ(correctNums, nums);
    ASSERT_TRUE(istr.eof());
}

// NOLINTNEXTLINE
TEST(TestStringstream, PocoJson) {
    AllocationCount::getAllocationCount() = 0;
    size_t start = AllocationCount::getAllocationCount().load();

    std::string_view jsonValue =
        R"json({"key1": "value1", "key2": "value2"})json";
    stringstream_view strv(jsonValue);

    ASSERT_EQ(AllocationCount::getAllocationCount().load(), start);

    std::istream istr(&strv);

    ASSERT_EQ(AllocationCount::getAllocationCount().load(), start);

    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(istr);

    Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

    ASSERT_EQ("value1", object->getValue<std::string>("key1"));
    ASSERT_EQ("value2", object->getValue<std::string>("key2"));
}
