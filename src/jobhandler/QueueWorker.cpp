#include "QueueWorker.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

auto job::QueueWorker::fork_process() -> pid_t {
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

auto job::QueueWorker::allocateJobOutputStream(
    const Poco::JSON::Object::Ptr &json)
    -> std::pair<std::fstream, std::fstream> {
    auto tmpdir = std::filesystem::temp_directory_path();
    std::string tmpfilename = "tmp" + json->getValue<std::string>("uuid");

    std::fstream joblog(tmpdir / tmpfilename,
                        std::ios::in | std::ios::out | std::ios::trunc);

    std::fstream joblogerr(tmpdir / (tmpfilename + ".stderr"),
                           std::ios::in | std::ios::out | std::ios::trunc);

    if (!joblog.is_open()) {
        LOGERR() << "fail to open " << (tmpdir / tmpfilename).string()
                 << std::endl;
    }

    if (!joblogerr.is_open()) {
        LOGERR() << "fail to open "
                 << (tmpdir / (tmpfilename + ".stderr")).string() << std::endl;
    }

    return std::pair<std::fstream, std::fstream>(std::move(joblog),
                                                 std::move(joblogerr));
}

auto job::QueueWorker::handle(const std::shared_ptr<QueueableJob> &newjob,
                              std::ostream &outstream,
                              std::ostream &errstream) {
    jobStatus result = noerror;
    ScopedStreamRedirect red(std::cout, outstream);
    ScopedStreamRedirect redcerr(std::cerr, errstream);

    try {
        newjob->handle();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        result = errexcept;
    }

    std::cout.flush();
    std::cerr.flush();
    return result;
}

auto job::QueueWorker::saveJobLog(std::fstream &outstream,
                                  std::fstream &errstream,
                                  GenericQueue::datamap_t &datamap) {
    try {
        putLogInDataMap(errstream, "JobStderr", datamap);
    } catch (const std::exception &e) {
        LOGERR() << e.what() << '\n';
        datamap["LastException"] = e.what();
    }

    try {
        putLogInDataMap(outstream, "JobStdout", datamap);
    } catch (const std::exception &e) {
        LOGERR() << e.what() << '\n';
        datamap["LastException"] = e.what();
    }
}

auto job::QueueWorker::handle_job_run(
    const std::shared_ptr<QueueableJob> &newjob,
    const Poco::JSON::Object::Ptr &json, GenericQueue::datamap_t &datamap)
    -> jobStatus {
    jobStatus result = noerror;
    LOGINFO() << "Job " << newjob->getName() << " tries " << newjob->tries
              << std::endl;

    newjob->tries++;

    auto [joblog, joblogerr] = allocateJobOutputStream(json);

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
            result = handle(newjob, joblog, joblogerr);
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

    saveJobLog(joblog, joblogerr, datamap);

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
        LOGERR() << datamap["payload"] << std::endl;
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

auto job::QueueWorker::waitpid(pid_t pid) -> pid_t {
    int status = 0;
    ::waitpid(pid, &status, 0);

    if (WIFEXITED(status)) { // NOLINT
                             // NOLINTNEXTLINE
        LOGINFO() << "exited, status=" << WEXITSTATUS(status) << std::endl;
        return WEXITSTATUS(status); // NOLINT
    }
    if (WIFSIGNALED(status)) { // NOLINT
        LOGINFO() << "killed by signal %d\n"
                  << WTERMSIG(status) << std::endl; // NOLINT
    } else if (WIFSTOPPED(status)) {                // NOLINT
        LOGINFO() << "stopped by signal %d\n"
                  << WSTOPSIG(status) << std::endl; // NOLINT
    } else if (WIFCONTINUED(status)) {              // NOLINT
        LOGINFO() << "continued" << std::endl;
    }

    return -1;
}

auto job::QueueWorker::pop(const std::string &queue, int timeout)
    -> std::string {
    auto data = queueServiceInst->pop(queue, timeout);
    return data.value_or(std::string());
}

job::QueueWorker::~QueueWorker() = default;
