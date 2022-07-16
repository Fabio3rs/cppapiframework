#include "RedisQueue.hpp"
#include "../utils/RedisService.hpp"
#include <Poco/Redis/Array.h>
#include <Poco/Redis/Command.h>
#include <Poco/Redis/Type.h>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <stdexcept>
#include <string>

void RedisQueue::push(const std::string &queue, const std::string &data) {
    RedisService::default_inst().rpush({aliasname + queue, data});
}

void RedisQueue::pushToLater(const std::string &queue, const std::string &data,
                             std::chrono::system_clock::time_point timep) {
    auto conn = RedisService::default_inst().get_connection();

    if (!conn) {
        throw std::runtime_error("Redis connection failed");
    }

    Poco::Redis::Command cmd("zadd");

    cmd << (aliasname + queue + ":later");
    cmd << std::to_string(std::chrono::system_clock::to_time_t(timep));
    cmd << data;

    if (conn->execute<int64_t>(cmd) == 0) {
        throw std::runtime_error("Job not added");
    }
}

auto RedisQueue::pop(const std::string &queue, int timeout)
    -> std::optional<std::string> {
    auto conn = RedisService::default_inst().get_connection();

    if (!conn) {
        return std::nullopt;
    }

    auto defaultQueue = aliasname + queue;
    auto ret = RedisService::blpop(*conn, {defaultQueue}, timeout);
    if (ret) {
        return {ret.value().second};
    }

    std::string popScript = R"lua(
local expired = redis.call('zrangebyscore', KEYS[1], '-inf', ARGV[1], 'LIMIT', 0, 1)

if (expired ~= nil) then
    redis.call('zremrangebyrank', KEYS[1], 0, 0)
end

return expired
)lua";

    auto now = std::to_string(time(nullptr));
    auto laterQueue = (defaultQueue + ":later");

    auto result = RedisService::eval<Poco::Redis::Array>(*conn, popScript,
                                                         {laterQueue}, {now});

    if (!result.isNull() && result.size() > 0) {
        auto jobuuid = result.get<Poco::Redis::BulkString>(0);

        if (!jobuuid.isNull()) {
            return {jobuuid.value()};
        }
    }

    return std::nullopt;
}

auto RedisQueue::getName() const -> std::string { return "redis"; }
void RedisQueue::setName(const std::string & /*ununsed*/) {}

auto RedisQueue::isConnected() const -> bool {
    return RedisService::default_inst().get_connection()->isConnected();
}

auto RedisQueue::getPersistentData(const std::string &name) const
    -> std::unordered_map<std::string, std::string> {
    return RedisService::default_inst().hgetall(aliasname + name);
}

void RedisQueue::setPersistentData(
    const std::string &name,
    const std::unordered_map<std::string, std::string> &data) {

    RedisService::default_inst().hset(aliasname + name, data);
}

void RedisQueue::delPersistentData(const std::string &name) {
    RedisService::default_inst().del({aliasname + name});
}

auto RedisQueue::expire(const std::string &name, int64_t seconds) -> bool {
    return RedisService::default_inst().cmd<int64_t>("expire", aliasname + name,
                                                     seconds) != 0;
}

auto RedisQueue::ttl(const std::string &name) -> int64_t {
    return RedisService::default_inst().cmd<int64_t>("ttl", aliasname + name);
}

RedisQueue::RedisQueue() = default;

RedisQueue::~RedisQueue() = default;
