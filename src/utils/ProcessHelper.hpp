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

    ProcessHelper(/* args */);
    virtual ~ProcessHelper();
};
