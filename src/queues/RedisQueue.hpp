#pragma once
#include "GenericQueue.hpp"
#include <queue>
#include <unordered_map>

class RedisQueue : public GenericQueue {
  public:
    void push(const std::string &queue, const std::string &data) override;
    void pushToLater(const std::string &queue, const std::string &data,
                     std::chrono::system_clock::time_point tp) override;

    auto pop(const std::string &queue, int timeout)
        -> std::optional<std::string> override;

    auto getName() const -> std::string override;
    void setName(const std::string &name) override;

    auto isConnected() const -> bool override;

    RedisQueue();
    ~RedisQueue() override;
};
