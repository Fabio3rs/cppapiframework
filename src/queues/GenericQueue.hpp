#pragma once

#include "../stdafx.hpp"

class GenericQueue {
  private:
    /* data */
  public:
    virtual void push(const std::string &queue, const std::string &data) = 0;
    virtual void pushToLater(const std::string &queue, const std::string &data,
                             std::chrono::system_clock::time_point tp) = 0;

    virtual auto pop(const std::string &queue, int timeout)
        -> std::optional<std::string> = 0;

    virtual auto getName() const -> std::string = 0;
    virtual void setName(const std::string &name) = 0;

    virtual auto isConnected() const -> bool = 0;

    GenericQueue();
    virtual ~GenericQueue();
};
