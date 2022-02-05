#include "ProcessHelper.hpp"
#include "../utils/CLog.hpp"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

auto ProcessHelper::wait(pid_t pid, int flags) -> std::pair<waitStatuses, int> {
    int status = 0;
    int waitpidres = ::waitpid(pid, &status, flags);

    if (WIFEXITED(status)) {                  // NOLINT
        return {exited, WEXITSTATUS(status)}; // NOLINT
    }
    if (WIFSIGNALED(status)) {               // NOLINT
        return {signaled, WTERMSIG(status)}; // NOLINT
    } else if (WIFSTOPPED(status)) {         // NOLINT
        return {stopped, WSTOPSIG(status)};  // NOLINT
    } else if (WIFCONTINUED(status)) {       // NOLINT
        return {continued, 0};               // NOLINT
    }

    return {unknown, waitpidres};
}

auto ProcessHelper::fork() -> pid_t {
    auto result = ::fork();
    if (result == 0) {
        CLog::log().SignalFork();
    }
    return result;
}

ProcessHelper::ProcessHelper(/* args */) = default;

ProcessHelper::~ProcessHelper() = default;
