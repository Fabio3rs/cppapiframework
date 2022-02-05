#pragma once
#include "../stdafx.hpp"
#include <unistd.h>

class ProcessHelper {
  private:
    /* data */
  public:
    enum waitStatuses { exited, signaled, stopped, continued, unknown };

    virtual auto fork() -> pid_t;
    virtual auto wait(pid_t pid, int flags = 0) -> std::pair<waitStatuses, int>;

    virtual auto operator=(const ProcessHelper &) -> ProcessHelper & = default;
    virtual auto operator=(ProcessHelper &&) -> ProcessHelper & = default;

    ProcessHelper(const ProcessHelper &) = default;
    ProcessHelper(ProcessHelper &&) = default;

    ProcessHelper(/* args */);
    virtual ~ProcessHelper();
};
