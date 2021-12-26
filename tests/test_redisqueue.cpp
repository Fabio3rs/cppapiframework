#include "../src/queues/RedisQueue.hpp"
#include <gtest/gtest.h>

static const std::string queue_name = "test_queue_worker:queue:default";

static std::shared_ptr<RedisQueue> redisq(std::make_shared<RedisQueue>());

TEST(TestRedisQueue, IsConnected) {
    EXPECT_TRUE(redisq);
    EXPECT_TRUE(redisq->isConnected());
}

TEST(TestRedisQueue, PushPops) {
    EXPECT_TRUE(redisq);
    EXPECT_TRUE(redisq->isConnected());

    std::string data = "0123456789 qwertyuiop";

    redisq->push(queue_name, data);
    auto popdata = redisq->pop(queue_name, 1);

    EXPECT_TRUE(popdata);

    EXPECT_EQ(popdata.value_or(std::string()), data);
}
