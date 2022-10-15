#pragma once

#include "GenericQueue.hpp"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <queue>
#include <unordered_map>

struct ScheduledStdQueueData {
    std::string data;
    int64_t pos;

    auto operator>(const ScheduledStdQueueData &other) const -> bool {
        return pos > other.pos;
    }

    auto operator<(const ScheduledStdQueueData &other) const -> bool {
        return pos < other.pos;
    }
};

class StdQueue : public GenericQueue {
    std::unordered_map<std::string, std::deque<std::string>> queue_map;
    std::unordered_map<std::string, std::priority_queue<ScheduledStdQueueData>>
        later_queue_map;
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>>
        persistentdata;

  public:
    void push(const std::string &queue, const std::string &data) override;
    void pushToLater(const std::string &queue, const std::string &data,
                     std::chrono::system_clock::time_point timep) override;

    auto getFullQueue(const std::string &queue) const
        -> std::vector<std::string> override {
        const auto &queueInst = queue_map.at(queue);
        std::vector<std::string> result;
        result.reserve(queueInst.size());
        result.assign(queueInst.begin(), queueInst.end());

        return result;
    }

    auto pop(const std::string &queue, int timeout)
        -> std::optional<std::string> override;

    auto getName() const -> std::string override;
    void setName(const std::string &name) override;

    auto getNumQueues() const -> size_t { return queue_map.size(); }
    auto getQueueSize(const std::string &queue) const -> size_t {
        return queue_map.at(queue).size();
    }

    auto getLaterQueueSize(const std::string &queue) const -> size_t {
        return later_queue_map.at(queue).size();
    }

    auto getLaterQueueTop(const std::string &queue) const
        -> ScheduledStdQueueData {
        return later_queue_map.at(queue).top();
    }

    auto getPersistentData(const std::string &name) const
        -> std::unordered_map<std::string, std::string> override {
        return persistentdata.at(name);
    }

    [[nodiscard]] auto getPersistentDataField(const std::string &name,
                                              const std::string &field) const
        -> std::optional<std::string> override {
        auto val = persistentdata.at(name);

        if (auto fieldData = val.find(field); fieldData != val.end()) {
            return fieldData->second;
        }

        return {};
    }

    void setPersistentData(
        const std::string &name,
        const std::unordered_map<std::string, std::string> &data) override {
        auto &map = persistentdata[name];
        for (const auto &dinst : data) {
            map.insert_or_assign(dinst.first, dinst.second);
        }
    }

    void delPersistentData(const std::string &name) override {
        persistentdata.erase(name);
    }

    auto isConnected() const -> bool override { return true; }

    StdQueue(const StdQueue &) = default;
    auto operator=(const StdQueue &) -> StdQueue & = default;

    StdQueue(StdQueue &&) = default;
    auto operator=(StdQueue &&) -> StdQueue & = default;

    StdQueue();
    ~StdQueue() override;
};
