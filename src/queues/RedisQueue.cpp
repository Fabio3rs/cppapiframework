#include "RedisQueue.hpp"
#include "../utils/RedisService.hpp"

void RedisQueue::push(const std::string &queue, const std::string &data) {
    RedisService::default_inst().rpush({queue, data});
}
void RedisQueue::pushToLater(
    const std::string &queue, const std::string &data,
    std::chrono::system_clock::time_point /*ununsed*/) {
    RedisService::default_inst().rpush({queue, data});
}

auto RedisQueue::pop(const std::string &queue, int timeout)
    -> std::optional<std::string> {
    auto ret = RedisService::default_inst().blpop({queue}, timeout);
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

RedisQueue::RedisQueue() {}

RedisQueue::~RedisQueue() {}
