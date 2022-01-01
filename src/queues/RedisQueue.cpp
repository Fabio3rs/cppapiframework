#include "RedisQueue.hpp"
#include "../utils/RedisService.hpp"

void RedisQueue::push(const std::string &queue, const std::string &data) {
    RedisService::default_inst().rpush({aliasname + queue, data});
}

void RedisQueue::pushToLater(
    const std::string &queue, const std::string &data,
    std::chrono::system_clock::time_point /*ununsed*/) {
    RedisService::default_inst().rpush({aliasname + queue, data});
}

auto RedisQueue::pop(const std::string &queue, int timeout)
    -> std::optional<std::string> {
    auto ret = RedisService::default_inst().blpop({aliasname + queue}, timeout);
    if (!ret) {
        return std::nullopt;
    }
    return std::optional<std::string>(ret.value().second);
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
    return RedisService::default_inst().cmd<int64_t>("expire", name, seconds);
}

auto RedisQueue::ttl(const std::string &name) -> int64_t {
    return RedisService::default_inst().cmd<int64_t>("ttl", name);
}

RedisQueue::RedisQueue() {}

RedisQueue::~RedisQueue() {}
