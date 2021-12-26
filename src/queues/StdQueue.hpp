#pragma once
#include "GenericQueue.hpp"
#include <queue>
#include <unordered_map>

class StdQueue : public GenericQueue {
    std::unordered_map<std::string, std::queue<std::string>> queue_map;

  public:
    void push(const std::string &queue, const std::string &data) override;
    void pushToLater(const std::string &queue, const std::string &data,
                     std::chrono::system_clock::time_point tp) override;

    auto pop(const std::string &queue, int timeout)
        -> std::optional<std::string> override;

    auto getName() const -> std::string override;
    void setName(const std::string &name) override;

    auto getNumQueues() const -> size_t { return queue_map.size(); }
    auto getQueueSize(const std::string &queue) const -> size_t {
        return queue_map.at(queue).size();
    }

    auto isConnected() const -> bool override { return true; }

    StdQueue();
    ~StdQueue() override;
};
