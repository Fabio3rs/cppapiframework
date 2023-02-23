#pragma once

#include "../queues/GenericQueue.hpp"
#include "../stdafx.hpp"
#include "../utils/PocoJsonStringify.hpp"
#include "../utils/ProcessHelper.hpp"
#include "../utils/ScopedStreamRedirect.hpp"
#include "JobsHandler.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <utility>

namespace job {

class QueueWorker {
  protected:
    std::shared_ptr<JobsHandler> jobhandler;
    std::shared_ptr<GenericQueue> queueServiceInst;
    std::shared_ptr<ProcessHelper> processHelperInst;

    int queueTimeout{1}, retryInTimeout{0};
    int64_t jobLogExpireSeconds{3600};

    std::atomic<bool> running{true};
    std::atomic<bool> forkToHandle{false};
    std::atomic<bool> cleanSuccessfulJobsLogs{true};

    static auto allocateJobOutputStream(const Poco::JSON::Object::Ptr &json)
        -> std::pair<std::fstream, std::fstream>;

  public:
    void setProcessHelper(std::shared_ptr<ProcessHelper> pHelper) {
        processHelperInst = std::move(pHelper);
    }

    auto getProcessHelper() -> std::shared_ptr<ProcessHelper> {
        if (!processHelperInst) {
            processHelperInst = std::make_shared<ProcessHelper>();
        }

        return processHelperInst;
    }

    virtual auto
    process_retry_condition(const std::shared_ptr<QueueableJob> &job)
        -> jobStatus;

    auto handle_job_run(const std::shared_ptr<QueueableJob> &newjob,
                        const Poco::JSON::Object::Ptr &json,
                        GenericQueue::datamap_t &datamap,
                        const std::string &jobname) -> jobStatus;

    /**
     * @brief instanciates and run the job
     * @todo improve the error catching design, log the tries
     *
     * param queue queue name
     * @param datamap all job data
     * @return jobStatus job result
     */
    auto work(const std::string & /*queue*/, GenericQueue::datamap_t &datamap,
              const std::string &jobname) -> jobStatus;

    auto fork_process() -> pid_t;

    /**
     * @brief Adds a job to the queue
     *
     * @tparam T job type
     * @param queue queue name
     * @param job job instance
     * @return std::string job uuid
     */
    template <class T>
    auto push(const std::string &queue, const T &job,
              std::chrono::system_clock::time_point scheduledAt = {})
        -> std::string {
        auto json = jobhandler->create_jobpayload(job);
        constexpr auto DEFAULT_UUID_SIZE = 36;
        constexpr size_t KEYSIZE = sizeof("job_instance:") + DEFAULT_UUID_SIZE;
        std::string jobuuid =
            Poco::UUIDGenerator::defaultGenerator().createOne().toString();

        std::string persistentkey;
        persistentkey.reserve(KEYSIZE);
        persistentkey = "job_instance:";
        persistentkey += jobuuid;

        json->set("uuid", jobuuid);

        queueServiceInst->setPersistentData(
            persistentkey, {{"tries", "0"},
                            {"maxtries", std::to_string(job.getMaxTries())},
                            {"payload", PocoJsonStringify::JsonToString(json)},
                            {"created_at_unixt", std::to_string(time(nullptr))},
                            {"className", std::string(job.getName())}});

        if (scheduledAt.time_since_epoch().count() != 0) {
            queueServiceInst->pushToLater(queue, persistentkey, scheduledAt);
        } else {
            queueServiceInst->push(queue, persistentkey);
        }

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

        int64_t retryAfter = 0;

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
            try {
                if (auto retryAfterOpt =
                        queueServiceInst->getPersistentDataField(
                            jobname, "retryAfter")) {
                    retryAfter = std::stoll(retryAfterOpt.value());
                }
            } catch (...) {
            }

            queueServiceInst->setPersistentData(jobname, datamap);

            if (retryAfter == 0) {
                queueServiceInst->push(queue, jobname);
            } else {
                queueServiceInst->pushToLater(
                    queue, jobname,
                    std::chrono::system_clock::now() +
                        std::chrono::seconds(retryAfter));
            }
            break;
        }
    }

    auto getNumberOfPendentJobTypes(const std::string &queue)
        -> std::unordered_map<std::string, size_t> {
        auto jobList = queueServiceInst->getFullQueue(queue);
        std::unordered_map<std::string, size_t> result;

        for (const auto &jobName : jobList) {
            auto jobData = queueServiceInst->getPersistentData(jobName);

            ++result[jobData.at("className")];
        }

        return result;
    }

    template <class T>
    static auto
    getNumberOfJobsByT(const std::unordered_map<std::string, size_t> &data)
        -> size_t {
        return data.at(job::JobsHandler::getTypeName<T>());
    }

    auto do_one(const std::string &queue, const std::string &jobname) -> bool;

    /**
     * @brief Pop job from the queue and run it
     *
     * @param queue queue name
     * @return true has job in the queue and runned it
     * @return false queue empty
     */
    auto do_one(const std::string &queue) -> bool;

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

    virtual auto operator=(const QueueWorker &) -> QueueWorker & = delete;
    virtual auto operator=(QueueWorker &&) -> QueueWorker & = delete;

    QueueWorker(const QueueWorker &) = delete;
    QueueWorker(QueueWorker &&) = delete;

    QueueWorker(std::shared_ptr<JobsHandler> jobh,
                std::shared_ptr<GenericQueue> queueService);
    virtual ~QueueWorker();
};

} // namespace job
