#include "../projstdafx.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

auto job::QueueWorker::fork_process() -> pid_t {
    if (forkToHandle) {
        return getProcessHelper()->fork();
    }

    return 0;
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
        STDLOGERR() << "fail to open " << (tmpdir / tmpfilename).string()
                    << std::endl;
    }

    if (!joblogerr.is_open()) {
        STDLOGERR() << "fail to open "
                    << (tmpdir / (tmpfilename + ".stderr")).string()
                    << std::endl;
    }

    return std::make_pair(std::move(joblog), std::move(joblogerr));
}

auto job::QueueWorker::handle_job_run(
    const std::shared_ptr<QueueableJob> &newjob,
    const Poco::JSON::Object::Ptr &json, GenericQueue::datamap_t &datamap,
    const std::string &jobname) -> jobStatus {
    jobStatus result = noerror;
    STDLOGINFO() << "Job " << newjob->getName() << " tries " << newjob->tries
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
            result = jobhandler->handle(newjob, joblog, joblogerr);
        } break;

        default: {
            auto resPid = getProcessHelper()->wait(pid, 0);

            if (resPid.first == ProcessHelper::waitStatuses::exited &&
                resPid.second == 0) {
                result = noerror;
            } else {
                STDLOGINFO() << "wait status: " << resPid.first
                             << " exit code: " << resPid.second << std::endl;
                result = process_retry_condition(newjob);
            }
        } break;
        }
    } catch (const std::exception &e) {
        datamap["LastException"] = e.what();
        std::cerr << e.what() << '\n';
        result = errexcept;
    }

    try {
        if (result != noerror && newjob->getRetryAfterSecs() > 0) {
            queueServiceInst->setPersistentData(
                jobname,
                {{"retryAfter", std::to_string(newjob->getRetryAfterSecs())}});
        }
    } catch (const Poco::Exception &e) {
        STDLOGERR() << e.displayText() << std::endl;
    } catch (const std::exception &e) {
        STDLOGERR() << e.what() << std::endl;
    }

    if (forkToHandle && pid == 0) {
        /**
         * @brief Forked process exit with result
         *
         */
        STDLOGINFO() << "Exiting job fork with status: " << result << std::endl;
        exit(result);
    }

    if (auto retryAfter = datamap.find("retryAfter");
        retryAfter != datamap.end()) {
        datamap.erase(retryAfter);
    }

    jobhandler->saveJobLog(joblog, joblogerr, datamap);

    return result;
}

auto job::QueueWorker::work(const std::string & /*queue*/,
                            GenericQueue::datamap_t &datamap,
                            const std::string &jobname) -> jobStatus {
    std::shared_ptr<QueueableJob> newjob;
    jobStatus result = noerror;

    /**
     * @brief allocate and construct the instance
     *
     */
    Poco::JSON::Object::Ptr json;

    try {
        STDLOGINFO() << "Instancing job " << datamap.at("className")
                     << std::endl;

        Poco::JSON::Parser parser;
        json =
            parser.parse(datamap["payload"]).extract<Poco::JSON::Object::Ptr>();

    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        STDLOGERR() << datamap["payload"] << std::endl;
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
        result = handle_job_run(newjob, json, datamap, jobname);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        result = errexcept;
    }

    if (result != noerror) {
        result = process_retry_condition(newjob);
        STDLOGINFO() << "Job " << newjob->getName() << " error " << result
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

auto job::QueueWorker::pop(const std::string &queue, int timeout)
    -> std::string {
    auto data = queueServiceInst->pop(queue, timeout);
    return data.value_or(std::string());
}

job::QueueWorker::~QueueWorker() = default;

job::QueueWorker::QueueWorker(std::shared_ptr<JobsHandler> jobh,
                              std::shared_ptr<GenericQueue> queueService)
    : jobhandler(std::move(jobh)), queueServiceInst(std::move(queueService)) {}

auto job::QueueWorker::process_retry_condition(
    const std::shared_ptr<QueueableJob> &job) -> jobStatus {
    if (job->getMaxTries() == 0) {
        return errorretry;
    }

    if (job->getTries() < job->getMaxTries()) {
        return errorretry;
    }

    return errorremove;
}

auto job::QueueWorker::do_one(const std::string &queue) -> bool {
    auto jobpayload = pop(queue, queueTimeout);

    if (jobpayload.empty()) {
        return false;
    }

    return do_one(queue, jobpayload);
}

auto job::QueueWorker::do_one(const std::string &queue,
                              const std::string &jobname) -> bool {
    if (jobname.empty()) {
        return false;
    }

    STDLOGINFO() << "Job UUID name " << jobname << std::endl;
    auto jobdata = queueServiceInst->getPersistentData(jobname);

    jobStatus workresult = work(queue, jobdata, jobname);

    process_job_result(queue, jobname, jobdata, workresult);

    return true;
}
