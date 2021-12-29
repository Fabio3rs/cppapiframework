#include "../src/utils/RedisService.hpp"
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <gtest/gtest.h>

static std::string test_hashset_key =
    "test_hashset:" +
    Poco::UUIDGenerator::defaultGenerator().createRandom().toString();

TEST(TestRedisService, IsConnected) {
    EXPECT_TRUE(RedisService::default_inst().get_connection());
}

TEST(TestRedisService, hsetReturnsOne) {
    std::cout << "Using redis key " << test_hashset_key << std::endl;
    EXPECT_EQ(RedisService::default_inst().hset(
                  test_hashset_key,
                  {{"firstkey", "firstdata"}, {"secondkey", "seconddata"}}),
              2);

    EXPECT_EQ(RedisService::default_inst().del({test_hashset_key}), 1);
}

TEST(TestRedisService, hgetReturnsOne) {
    std::cout << "Using redis key " << test_hashset_key << std::endl;
    EXPECT_EQ(RedisService::default_inst().hset(
                  test_hashset_key,
                  {{"firstkey", "firstdata"}, {"secondkey", "seconddata"}}),
              2);

    auto hgetallresult = RedisService::default_inst().hgetall(test_hashset_key);

    EXPECT_EQ(
        RedisService::default_inst()
            .hget<Poco::Nullable<std::string>>(test_hashset_key, "firstkey")
            .value(std::string()),
        "firstdata");
    EXPECT_EQ(
        RedisService::default_inst()
            .hget<Poco::Nullable<std::string>>(test_hashset_key, "secondkey")
            .value(std::string()),
        "seconddata");

    EXPECT_EQ(RedisService::default_inst().del({test_hashset_key}), 1);
}

TEST(TestRedisService, hgetallReturnsEverything) {
    std::cout << "Using redis key " << test_hashset_key << std::endl;
    EXPECT_EQ(RedisService::default_inst().hset(
                  test_hashset_key,
                  {{"firstkey", "firstdata"}, {"secondkey", "seconddata"}}),
              2);

    auto hgetallresult = RedisService::default_inst().hgetall(test_hashset_key);

    EXPECT_NO_THROW(EXPECT_EQ(hgetallresult.at("firstkey"), "firstdata"));
    EXPECT_NO_THROW(EXPECT_EQ(hgetallresult.at("secondkey"), "seconddata"));

    EXPECT_EQ(RedisService::default_inst().del({test_hashset_key}), 1);
}
