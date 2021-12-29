#include "QueueWorker.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t job::QueueWorker::fork_process() {
    if (forkToHandle) {
        return fork();
    }

    return 0;
}

pid_t job::QueueWorker::waitpid(pid_t pid) {
    int status = 0;
    ::waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        printf("exited, status=%d\n", WEXITSTATUS(status));
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        printf("killed by signal %d\n", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        printf("stopped by signal %d\n", WSTOPSIG(status));
    } else if (WIFCONTINUED(status)) {
        printf("continued\n");
    }

    return -1;
}

/*void job::QueueWorker::push(const std::string &queue,
                            const Poco::JSON::Object::Ptr &json) {
    std::stringstream sstr;
    json->stringify(sstr);

    queueServiceInst->push(queue, sstr.str());
}*/

auto job::QueueWorker::pop(const std::string &queue, int timeout)
    -> std::string {
    auto data = queueServiceInst->pop(queue, timeout);
    return data.value_or(std::string());
}

job::QueueWorker::~QueueWorker() {}
