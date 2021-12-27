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
     * @param json json payload
     * @return jobStatus job result
     */
    jobStatus work(const std::string & /*queue*/,
                   Poco::JSON::Object::Ptr &json) {
        std::shared_ptr<QueueableJob> newjob;
        jobStatus result = noerror;

        /**
         * @brief allocate and construct the instance
         *
         */
        try {
            LOGINFO() << "Instancing job "
                      << json->getValue<std::string>("className") << std::endl;
            newjob = jobhandler->instance_from_payload(json);
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            return errorremove;
        }

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
            auto tmpptr = jobhandler->recreate_jobpayload(json, *newjob);

            if (tmpptr) {
                json = tmpptr;
            }
        }

        return result;
    }

    pid_t fork_process();
    pid_t waitpid(pid_t pid);

    void push(const std::string &queue, const Poco::JSON::Object::Ptr &json);
    auto pop(const std::string &queue, int timeout) -> Poco::JSON::Object::Ptr;

    /**
     * @brief Process the job result, re-adds in the queue if is errorretry
     *
     * @param queue queue name
     * @param jobpayload job json payload
     * @param workresult job run result
     */
    void process_job_result(const std::string &queue,
                            const Poco::JSON::Object::Ptr &jobpayload,
                            jobStatus workresult) {
        switch (workresult) {
        case noerror:
            /* code */
            break;

        case errorremove:
            break;

        case errorretry:
            push(queue, jobpayload);
            break;
        }
    }

    bool do_one(const std::string &queue, Poco::JSON::Object::Ptr &jobpayload) {
        if (jobpayload.isNull()) {
            return false;
        }

        jobStatus workresult = work(queue, jobpayload);

        process_job_result(queue, jobpayload, workresult);

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

        if (jobpayload.isNull()) {
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
