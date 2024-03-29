#include "projstdafx.hpp"

#include <gtest/gtest.h>

static const std::string queue_name = "test_queue_worker:queue:default";

static std::shared_ptr<RedisQueue> redisq(std::make_shared<RedisQueue>());

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisQueue, IsConnected) {
    EXPECT_TRUE(redisq);
    EXPECT_TRUE(redisq->isConnected());
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisQueue, PushPops) {
    EXPECT_TRUE(redisq);
    EXPECT_TRUE(redisq->isConnected());

    std::string data = "0123456789 qwertyuiop";

    redisq->push(queue_name, data);
    auto popdata = redisq->pop(queue_name, 1);

    EXPECT_TRUE(popdata);

    EXPECT_EQ(popdata.value_or(std::string()), data);
}

namespace {
void checkQueueContent(const std::string &data) {
    auto queueList = redisq->getFullQueue(queue_name);

    auto found = std::find(queueList.begin(), queueList.end(), data);

    for (const auto &val : queueList) {
        std::cout << val << std::endl;
    }

    EXPECT_NE(found, queueList.end());
    EXPECT_GT(queueList.size(), 0);
}
} // namespace

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisQueue, ExpireTTLQueue) {
    std::string data = "0123456789 qwertyuiop";

    redisq->push(queue_name, data);

    checkQueueContent(data);

    EXPECT_TRUE(redisq->expire(queue_name, 32));
    EXPECT_GT(redisq->ttl(queue_name), 10);
    EXPECT_LE(redisq->ttl(queue_name), 32);

    auto popdata = redisq->pop(queue_name, 1);
    EXPECT_TRUE(popdata);

    EXPECT_EQ(popdata.value_or(std::string()), data);
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestRedisQueue, PushLaterPops) {
    EXPECT_TRUE(redisq);
    EXPECT_TRUE(redisq->isConnected());

    std::string data = "0123456789 qwertyuiop for later";

    auto now = std::chrono::system_clock::now();

    auto custom_queue =
        queue_name +
        std::to_string(
            std::chrono::system_clock::now().time_since_epoch().count());

    redisq->pushToLater(custom_queue, data, now);
    auto popdata = redisq->pop(custom_queue, 1);

    EXPECT_TRUE(popdata);

    EXPECT_EQ(popdata.value_or(std::string()), data);
}
