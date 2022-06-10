#pragma once
#include "GenericQueue.hpp"
#include <queue>
#include <unordered_map>

class RedisQueue : public GenericQueue {
    std::string aliasname;

  public:
    void push(const std::string &queue, const std::string &data) override;
    void pushToLater(const std::string &queue, const std::string &data,
                     std::chrono::system_clock::time_point timep) override;

    auto pop(const std::string &queue, int timeout)
        -> std::optional<std::string> override;

    [[nodiscard]] auto getName() const -> std::string override;
    void setName(const std::string &name) override;

    [[nodiscard]] auto getPersistentData(const std::string &name) const
        -> std::unordered_map<std::string, std::string> override;

    void setPersistentData(
        const std::string &name,
        const std::unordered_map<std::string, std::string> &data) override;

    void delPersistentData(const std::string &name) override;

    [[nodiscard]] auto isConnected() const -> bool override;

    auto expire(const std::string &name, int64_t seconds) -> bool override;
    auto ttl(const std::string &name) -> int64_t override;

    RedisQueue(const RedisQueue &) = default;
    auto operator=(const RedisQueue &) -> RedisQueue & = default;

    RedisQueue(RedisQueue &&) = default;
    auto operator=(RedisQueue &&) -> RedisQueue & = default;

    RedisQueue();
    ~RedisQueue() override;
};
