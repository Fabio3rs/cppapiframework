#include "StdQueue.hpp"

void StdQueue::push(const std::string &queue, const std::string &data) {
    queue_map[queue].push_back(data);
}

void StdQueue::pushToLater(const std::string &queue, const std::string &data,
                           std::chrono::system_clock::time_point /*ununsed*/) {
    queue_map[queue].push_back(data);
}

auto StdQueue::pop(const std::string &queue, int /**/)
    -> std::optional<std::string> {
    auto &queueInst = queue_map[queue];

    if (queueInst.empty()) {
        return std::nullopt;
    }

    auto front = queueInst.front();
    queueInst.pop_front();
    return {front};
}

auto StdQueue::getName() const -> std::string { return "in_memory"; }
void StdQueue::setName(const std::string & /*ununsed*/) {}

StdQueue::StdQueue() = default;

StdQueue::~StdQueue() = default;
