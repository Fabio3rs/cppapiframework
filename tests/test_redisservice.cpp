#include "projstdafx.hpp"

#include <gtest/gtest.h>

static std::string test_hashset_key =
    "test_hashset:" +
    Poco::UUIDGenerator::defaultGenerator().createRandom().toString();

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisService, IsConnected) {
    EXPECT_TRUE(RedisService::default_inst().get_connection());
}

static void default_hset_key_data() {
    EXPECT_EQ(RedisService::default_inst().hset(
                  test_hashset_key,
                  {{"firstkey", "firstdata"}, {"secondkey", "seconddata"}}),
              2);
}

static void default_key_test_delete() {
    EXPECT_EQ(RedisService::default_inst().del({test_hashset_key}), 1);
}

static void test_eq_hset_firstkey(
    const std::unordered_map<std::string, std::string> &hgetallresult) {
    EXPECT_EQ(hgetallresult.at("firstkey"), "firstdata");
}

static void test_eq_hset_secondkey(
    const std::unordered_map<std::string, std::string> &hgetallresult) {
    EXPECT_EQ(hgetallresult.at("secondkey"), "seconddata");
}

static void test_eq_hset_default(
    const std::unordered_map<std::string, std::string> &hgetallresult) {
    // NOLINTNEXTLINE(hicpp-avoid-goto)
    EXPECT_NO_THROW(test_eq_hset_firstkey(hgetallresult));
    // NOLINTNEXTLINE(hicpp-avoid-goto)
    EXPECT_NO_THROW(test_eq_hset_secondkey(hgetallresult));
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisService, hsetReturnsOne) {
    std::cout << "Using redis key " << test_hashset_key << std::endl;
    default_hset_key_data();

    default_key_test_delete();
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisService, hgetReturnsOne) {
    std::cout << "Using redis key " << test_hashset_key << std::endl;
    default_hset_key_data();

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

    default_key_test_delete();
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisService, hdelTest) {
    std::cout << "Using redis key " << test_hashset_key << std::endl;
    default_hset_key_data();

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

    EXPECT_EQ(RedisService::default_inst().hdel(test_hashset_key, {"firstkey"}),
              1);
    EXPECT_EQ(
        RedisService::default_inst().hdel(test_hashset_key, {"secondkey"}), 1);
    EXPECT_EQ(RedisService::default_inst().hdel(test_hashset_key, {"firstkey"}),
              0);

    EXPECT_EQ(RedisService::default_inst().del({test_hashset_key}),
              0); // Se não houverem mais valores isso deve retornar 0 pois a
                  // key também não existe mais
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisService, hgetallReturnsEverything) {
    std::cout << "Using redis key " << test_hashset_key << std::endl;
    default_hset_key_data();

    auto hgetallresult = RedisService::default_inst().hgetall(test_hashset_key);
    test_eq_hset_default(hgetallresult);

    default_key_test_delete();
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisService, customCmdExpireTTL) {
    std::cout << "Using redis key " << test_hashset_key << std::endl;
    default_hset_key_data();

    EXPECT_EQ(RedisService::default_inst().cmd<int64_t>("expire",
                                                        test_hashset_key, 32),
              1);

    EXPECT_GT(
        RedisService::default_inst().cmd<int64_t>("ttl", test_hashset_key), 10);
    EXPECT_LE(
        RedisService::default_inst().cmd<int64_t>("ttl", test_hashset_key), 32);

    default_key_test_delete();
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisService, jsonSetGet) {
    auto conn = RedisService::default_inst().get_connection();

    if (!conn) {
        throw std::runtime_error("conn is null");
    }

    {
        Poco::JSON::Object::Ptr json = new Poco::JSON::Object;

        json->set("test", "a");
        json->set("test2", 0);
        json->set("test3", false);

        EXPECT_EQ(
            RedisService::setJson(*conn, "redisservice:testsetjson", json, 60),
            "OK");
    }

    auto retJson = RedisService::getJson(*conn, "redisservice:testsetjson")
                       .extract<Poco::JSON::Object::Ptr>();

    EXPECT_FALSE(retJson.isNull());

    EXPECT_EQ(retJson->getValue<std::string>("test"), "a");
    EXPECT_EQ(retJson->getValue<int>("test2"), 0);
    EXPECT_EQ(retJson->getValue<bool>("test3"), false);
}
