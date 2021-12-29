#pragma once

#include "../queues/GenericQueue.hpp"
#include "../stdafx.hpp"
#include "../utils/LogDefines.hpp"
#include "../utils/ScopedStreamRedirect.hpp"
#include "JobsHandler.hpp"
#include <fstream>
#include <unistd.h>

namespace job {

enum jobStatus { noerror, errorremove, errorretry };

class QueueWorker {
    std::shared_ptr<JobsHandler> jobhandler;
    std::shared_ptr<GenericQueue> queueServiceInst;

  protected:
    int queueTimeout{1}, retryInTimeout{0};
    std::atomic<bool> running{true};
    std::atomic<bool> forkToHandle{false};

  public:
    jobStatus
    process_retry_condition(const std::shared_ptr<QueueableJob> &job) {
        if (job->getMaxTries() == 0) {
            return errorretry;
        }

        if (job->getTries() < job->getMaxTries()) {
            return errorretry;
        }

        return errorremove;
    }

    /**
     * @brief instanciates and run the job
     * @todo improve the error catching design, log the tries
     *
     * param queue queue name
     * @param datamap all job data
     * @return jobStatus job result
     */
    jobStatus work(const std::string & /*queue*/,
                   std::unordered_map<std::string, std::string> &datamap) {
        std::shared_ptr<QueueableJob> newjob;
        jobStatus result = noerror;

        /**
         * @brief allocate and construct the instance
         *
         */
        Poco::JSON::Object::Ptr json;
        try {
            LOGINFO() << "Instancing job " << datamap.at("className")
                      << std::endl;

            Poco::JSON::Parser parser;
            json = parser.parse(datamap["payload"])
                       .extract<Poco::JSON::Object::Ptr>();

            auto dataobj = json->getObject("data");
            dataobj->set("tries", std::stoull(datamap["tries"]));
            dataobj->set("maxtries", std::stoull(datamap["maxtries"]));

            newjob = jobhandler->instance_from_payload(json);
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            return errorremove;
        }

        datamap.erase("payload");

        if (!newjob) {
            return errorretry;
        }

        pid_t pid = -1;
        try {
            LOGINFO() << "Job " << newjob->getName() << " tries "
                      << newjob->tries << std::endl;

            newjob->tries++;
            std::fstream joblog(json->getValue<std::string>("uuid"),
                                std::ios::in | std::ios::out | std::ios::trunc);

            /**
             * @brief Fork the process if the flag is enabled
             *
             */
            pid = fork_process();

            switch (pid) {
            case -1:
                throw std::runtime_error("Fork failed");
                break;

            case 0: {
                ScopedStreamRedirect red(std::cout, joblog);
                newjob->handle();
                std::cout.flush();
            } break;

            default:
                result = waitpid(pid) == 0 ? noerror
                                           : process_retry_condition(newjob);
                if (forkToHandle) {
                    std::string line;
                    joblog.seekg(std::ios::beg);
                    while (std::getline(joblog, line)) {
                        std::cout << "JOB LINE " << line << std::endl;
                    }
                }

                break;
            }
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            result = process_retry_condition(newjob);
        }

        if (forkToHandle && pid == 0) {
            /**
             * @brief Forked process exit with result
             *
             */
            exit(result);
        }

        if (result != noerror) {
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

    pid_t fork_process();
    pid_t waitpid(pid_t pid);

    template <class T> void push(const std::string &queue, const T &job) {
        auto json = jobhandler->create_jobpayload(job);
        constexpr size_t KEYSIZE = sizeof("job_instance:") + 36;
        std::string jobuuid =
            Poco::UUIDGenerator::defaultGenerator().createOne().toString();

        std::string persistentkey;
        persistentkey.reserve(KEYSIZE);
        persistentkey = "job_instance:";
        persistentkey += jobuuid;

        json->set("uuid", jobuuid);

        std::stringstream sstr;
        json->stringify(sstr);

        queueServiceInst->setPersistentData(
            persistentkey, {{"tries", "0"},
                            {"maxtries", std::to_string(job.getMaxTries())},
                            {"payload", sstr.str()},
                            {"created_at_unixt", std::to_string(time(nullptr))},
                            {"className", std::string(job.getName())}});

        queueServiceInst->push(queue, persistentkey);
    }

    // void push(const std::string &queue, const Poco::JSON::Object::Ptr &json);
    auto pop(const std::string &queue, int timeout) -> std::string;

    /**
     * @brief Process the job result, re-adds in the queue if is errorretry
     *
     * @param queue queue name
     * @param jobname name of the job to the queue
     * @param workresult job run result
     */
    void process_job_result(
        const std::string &queue, const std::string &jobname,
        const std::unordered_map<std::string, std::string> &datamap,
        jobStatus workresult) {
        switch (workresult) {
        case noerror:
            queueServiceInst->delPersistentData(jobname);
            break;

        case errorremove:
            break;

        case errorretry:
            queueServiceInst->setPersistentData(jobname, datamap);
            queueServiceInst->push(queue, jobname);
            break;
        }
    }

    bool do_one(const std::string &queue, std::string &jobname) {
        if (jobname.empty()) {
            return false;
        }

        LOGINFO() << "Job UUID name " << jobname << std::endl;
        auto jobdata = queueServiceInst->getPersistentData(jobname);

        jobStatus workresult = work(queue, jobdata);

        process_job_result(queue, jobname, jobdata, workresult);

        return true;
    }

    /**
     * @brief Pop job from the queue and run it
     *
     * @param queue queue name
     * @return true has job in the queue and runned it
     * @return false queue empty
     */
    bool do_one(const std::string &queue) {
        auto jobpayload = pop(queue, queueTimeout);

        if (jobpayload.empty()) {
            return false;
        }

        return do_one(queue, jobpayload);
    }

    /**
     * @brief Run jobs in loop until running is false
     *
     * @param queue queue name
     */
    void loop(const std::string &queue) {
        while (running) {
            do_one(queue);
        }
    }

    void setForkToHandleJob(bool forkstatus) { forkToHandle = forkstatus; }

    QueueWorker(std::shared_ptr<JobsHandler> jobh,
                std::shared_ptr<GenericQueue> queueService)
        : jobhandler(jobh), queueServiceInst(queueService) {}
    ~QueueWorker();
};

} // namespace job
