#pragma once

#include "../queues/GenericQueue.hpp"
#include "../stdafx.hpp"
#include "../utils/LogDefines.hpp"
#include "../utils/ScopedStreamRedirect.hpp"
#include "JobsHandler.hpp"
#include <fstream>
#include <unistd.h>

namespace job {

enum jobStatus { noerror, errorremove, errorretry, errexcept };

class QueueWorker {
  protected:
    std::shared_ptr<JobsHandler> jobhandler;
    std::shared_ptr<GenericQueue> queueServiceInst;

    int queueTimeout{1}, retryInTimeout{0};
    int64_t jobLogExpireSeconds{3600};

    std::atomic<bool> running{true};
    std::atomic<bool> forkToHandle{false};
    std::atomic<bool> cleanSuccessfulJobsLogs{true};

    static auto allocateJobOutputStream(const Poco::JSON::Object::Ptr &json)
        -> std::pair<std::fstream, std::fstream>;

    static auto handle(const std::shared_ptr<QueueableJob> &newjob,
                       std::ostream &outstream, std::ostream &errstream);

    static auto saveJobLog(std::fstream &outstream, std::fstream &errstream,
                           GenericQueue::datamap_t &datamap);

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

    auto handle_job_run(const std::shared_ptr<QueueableJob> &newjob,
                        const Poco::JSON::Object::Ptr &json,
                        GenericQueue::datamap_t &datamap) -> jobStatus;

    /**
     * @brief instanciates and run the job
     * @todo improve the error catching design, log the tries
     *
     * param queue queue name
     * @param datamap all job data
     * @return jobStatus job result
     */
    auto work(const std::string & /*queue*/, GenericQueue::datamap_t &datamap)
        -> jobStatus;

    pid_t fork_process();
    static pid_t waitpid(pid_t pid);

    /**
     * @brief Adds a job to the queue
     *
     * @tparam T job type
     * @param queue queue name
     * @param job job instance
     * @return std::string job uuid
     */
    template <class T>
    auto push(const std::string &queue, const T &job) -> std::string {
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

        return jobuuid;
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
            if (cleanSuccessfulJobsLogs) {
                queueServiceInst->delPersistentData(jobname);
            } else {
                queueServiceInst->setPersistentData(jobname, datamap);
                queueServiceInst->expire(jobname, jobLogExpireSeconds);
            }
            break;

        case errorremove:
            queueServiceInst->setPersistentData(jobname, datamap);
            queueServiceInst->expire(jobname, jobLogExpireSeconds);
            break;

        case errexcept:
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

    /**
     * @brief Should keep the completed jobs data in the persistent storage?
     *
     * @param value true keep the logs; false don't
     */
    void setCleanSuccessfulJobsLogs(bool value) {
        cleanSuccessfulJobsLogs = value;
    }

    /**
     * @brief Finished jobs should be keep in the persistent storage for how
     * many seconds after it's finished?
     *
     * @param value seconds in integer
     */
    void setJobFinishedExpireSeconds(int64_t value) {
        jobLogExpireSeconds = value;
    }

    QueueWorker(std::shared_ptr<JobsHandler> jobh,
                std::shared_ptr<GenericQueue> queueService)
        : jobhandler(jobh), queueServiceInst(queueService) {}
    ~QueueWorker();
};

} // namespace job
