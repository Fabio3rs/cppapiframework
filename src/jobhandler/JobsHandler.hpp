/**
 * @file JobsHandler.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief Cadastro de tipos de jobs, criação de payload em json,
 * instancialização das classes a partir de payloads json
 * @version 0.1
 * @date 2021-12-23
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#include "../stdafx.hpp"
#include "QueueableJob.hpp"

namespace job {

class JobsHandler {
  public:
    /**
     * @brief Get the Type Name object. Uses typeid name to get the typename in
     * compile time
     *
     * @tparam T the type
     * @return constexpr std::string_view the name of the type
     */
    template <class T> static constexpr std::string_view getTypeName() {
        return typeid(T).name();
    }

    template <class T>
    static constexpr std::string_view getTypeNameByInst(const T & /*ununsed*/) {
        return typeid(T).name();
    }

    /**
     * @brief the function type to instanciate a new job
     *
     */
    using job_instance_fn_t = std::function<std::shared_ptr<QueueableJob>()>;

    /**
     * @brief Adds de job to the handler list
     *
     * @tparam T job class type
     * @throws std::invalid_argument if it's already in the list
     */
    template <class T> void register_job_handler() {
        std::string jobname(getTypeName<T>());
        if (joblist.find(jobname) != joblist.end()) {
            throw std::invalid_argument("Job já cadastrado na lista: " +
                                        jobname);
        }

        joblist[jobname] = []() { return std::make_shared<T>(); };
    }

    /**
     * @brief Test if a job is registered by it's type
     *
     * @tparam T job type
     * @return true handler registered
     * @return false handler not registered
     */
    template <class T> auto is_job_registered() const -> bool {
        return is_job_registered_name(std::string(getTypeName<T>()));
    }

    /**
     * @brief Test if a job is registered by it's string type name
     *
     * @param jobname the type name in string
     * @return true handler registered
     * @return false handler not registered
     */
    auto is_job_registered_name(const std::string &jobname) const -> bool {
        return joblist.find(jobname) != joblist.end();
    }

    /**
     * @brief Returns the function that instances a new job
     *
     * @param jobname the job class name
     * @return job_instance_fn_t function
     */
    auto instance_job_fn(const std::string &jobname) const
        -> job_instance_fn_t {
        return joblist.at(jobname);
    }

    /**
     * @brief receives a json payload and instanciates the job class
     *
     * @param payload json payload
     * @return std::shared_ptr<QueueableJob> job class instance ptr
     */
    auto instance_from_payload(const Poco::JSON::Object::Ptr &payload) const
        -> std::shared_ptr<QueueableJob> {
        job_instance_fn_t fn =
            instance_job_fn(payload->getValue<std::string>("className"));

        auto job = fn();
        job->from_json(payload->getObject("data"));
        return job;
    }

    /**
     * @brief Create a job json payload from object
     *
     * @tparam T job type
     * @param job job instance
     * @return Poco::JSON::Object::Ptr it's json payload
     */
    template <class T>
    static auto create_jobpayload(const T &job) -> Poco::JSON::Object::Ptr {
        Poco::JSON::Object::Ptr result(new Poco::JSON::Object);

        result->set("className", std::string(getTypeName<T>()));
        result->set(
            "uuid",
            Poco::UUIDGenerator::defaultGenerator().createOne().toString());

        result->set("data", job.dump_json());

        return result;
    }

    template <class T>
    static auto recreate_jobpayload(const Poco::JSON::Object::Ptr &oldpayload,
                                    const T &job) -> Poco::JSON::Object::Ptr {
        Poco::JSON::Object::Ptr result(oldpayload);

        if (result) {
            result->set("data", job.dump_json());
        } else {
            result = create_jobpayload(job);
        }

        return result;
    }

    static auto default_instance() -> std::shared_ptr<JobsHandler>;

  private:
    /**
     * @brief The jobs list, pair first: class name; pair second: the function
     * that instanciates the class
     *
     */
    std::unordered_map<std::string, job_instance_fn_t> joblist;
};

} // namespace job
