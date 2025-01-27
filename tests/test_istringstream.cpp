#include "projstdafx.hpp"

#include "allocation_count.hpp"
#include <cstddef>
#include <gtest/gtest.h>

// NOLINTNEXTLINE
TEST(TestStringstream, Stream) {
    AllocationCount::getAllocationCount() = 0;
    size_t start = AllocationCount::getAllocationCount().load();

    std::array<int, 9> correctNums{123456, 123456, 789, 10, 11, 12, 13, 14, 15};
    stringstream_view strv("123456 123456 789 10 11 12 13 14 15");

    ASSERT_TRUE(AllocationCount::getAllocationCount().load() == start);

    std::istream istr(&strv);

    ASSERT_TRUE(AllocationCount::getAllocationCount().load() == start);

    std::array<int, 9> nums{};
    for (int &num : nums) {
        istr >> num;
    }

    ASSERT_TRUE(AllocationCount::getAllocationCount().load() == start);
    ASSERT_EQ(correctNums, nums);
    ASSERT_TRUE(istr.eof());
}
