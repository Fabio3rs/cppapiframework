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

static auto putLogInDataMap(std::fstream &joblog, const std::string &key,
                            GenericQueue::datamap_t &datamap) {
    std::string line;
    std::string fullstrlog;
    joblog.seekg(0, std::ios::end);

    std::streampos printsize = joblog.tellg();
    fullstrlog.reserve((printsize > 0 ? static_cast<size_t>(printsize) : 0) +
                       8);
    joblog.seekg(0, std::ios::beg);

    while (std::getline(joblog, line)) {
        std::cout << "JOB LINE " << line << std::endl;
        fullstrlog += line;
        fullstrlog += "\n";
    }

    datamap[key] = fullstrlog;
}

auto job::QueueWorker::handle_job_run(std::shared_ptr<QueueableJob> newjob,
                                      const Poco::JSON::Object::Ptr &json,
                                      GenericQueue::datamap_t &datamap)
    -> jobStatus {
    jobStatus result = noerror;
    LOGINFO() << "Job " << newjob->getName() << " tries " << newjob->tries
              << std::endl;

    newjob->tries++;
    std::fstream joblog(json->getValue<std::string>("uuid"),
                        std::ios::in | std::ios::out | std::ios::trunc);

    std::fstream joblogerr(json->getValue<std::string>("uuid") + ".stderr",
                           std::ios::in | std::ios::out | std::ios::trunc);

    /**
     * @brief Fork the process if the flag is enabled
     *
     */
    pid_t pid = fork_process();

    try {
        switch (pid) {
        case -1:
            throw std::runtime_error("Fork failed");

        case 0: {
            ScopedStreamRedirect red(std::cout, joblog);
            ScopedStreamRedirect redcerr(std::cerr, joblogerr);
            newjob->handle();
            std::cout.flush();
            std::cerr.flush();
        } break;

        default:
            result =
                waitpid(pid) == 0 ? noerror : process_retry_condition(newjob);
            break;
        }
    } catch (const std::exception &e) {
        datamap["LastException"] = e.what();
        std::cerr << e.what() << '\n';
        result = errexcept;
    }

    if (forkToHandle && pid == 0) {
        /**
         * @brief Forked process exit with result
         *
         */
        exit(result);
    }

    try {
        putLogInDataMap(joblogerr, "JobStderr", datamap);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        datamap["LastException"] = e.what();
    }

    try {
        putLogInDataMap(joblog, "JobStdout", datamap);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        datamap["LastException"] = e.what();
    }

    return result;
}

auto job::QueueWorker::work(const std::string & /*queue*/,
                            GenericQueue::datamap_t &datamap) -> jobStatus {
    std::shared_ptr<QueueableJob> newjob;
    jobStatus result = noerror;

    /**
     * @brief allocate and construct the instance
     *
     */
    Poco::JSON::Object::Ptr json;

    try {
        LOGINFO() << "Instancing job " << datamap.at("className") << std::endl;

        Poco::JSON::Parser parser;
        json =
            parser.parse(datamap["payload"]).extract<Poco::JSON::Object::Ptr>();

    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return errorremove;
    }

    try {
        auto dataobj = json->getObject("data");
        dataobj->set("tries", std::stoull(datamap["tries"]));
        dataobj->set("maxtries", std::stoull(datamap["maxtries"]));
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return errorremove;
    }

    try {
        newjob = jobhandler->instance_from_payload(json);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return errorretry;
    }

    datamap.erase("payload");

    if (!newjob) {
        return errorretry;
    }

    try {
        result = handle_job_run(newjob, json, datamap);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        result = errexcept;
    }

    if (result != noerror) {
        result = process_retry_condition(newjob);
        LOGINFO() << "Job " << newjob->getName() << " error " << result
                  << std::endl;
    }

    /**
     * @brief Refresh the payload
     *
     */
    if (result == errorretry) {
        datamap["tries"] = std::to_string(newjob->getTries());
        datamap["maxtries"] = std::to_string(newjob->getMaxTries());
    }

    return result;
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
