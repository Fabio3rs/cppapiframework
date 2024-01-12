#include "projstdafx.hpp"

#include <gtest/gtest.h>

// NOLINTNEXTLINE
TEST(CacheUtils, Cache) {
    auto conn = RedisService::default_inst().get_connection();

    ASSERT_TRUE(conn);

    cache_utils::Cache::prefix = Strutils::multi_concat(
        "apitestscache:",
        std::to_string(std::chrono::high_resolution_clock::now()
                           .time_since_epoch()
                           .count()),
        ":");

    constexpr std::string_view name = "cacheutilstest";

    EXPECT_EQ(
        RedisService::default_inst().cmd<int64_t>(
            "EXISTS", Strutils::multi_concat(cache_utils::Cache::prefix, name)),
        0);

    std::string randomData =
        Poco::UUIDGenerator::defaultGenerator().createOne().toString();
    int calledTimes = 0;
    auto callbackfn = [&]() -> std::string {
        ++calledTimes;
        return randomData;
    };

    EXPECT_EQ(
        cache_utils::Cache::getAndOrSetCache(std::string(name), callbackfn, 10),
        randomData);

    // No data, so calling the callbackFn and increasing 1 to calledTimes
    EXPECT_EQ(calledTimes, 1);
    EXPECT_EQ(
        RedisService::default_inst().cmd<int64_t>(
            "EXISTS", Strutils::multi_concat(cache_utils::Cache::prefix, name)),
        1);

    EXPECT_EQ(
        cache_utils::Cache::getAndOrSetCache(std::string(name), callbackfn, 10),
        randomData);

    // No increments to calledTimes
    EXPECT_EQ(calledTimes, 1);
}
