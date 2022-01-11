/**
 * @file QueueableJob.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief Classe base para os jobs
 * @version 0.1
 * @date 2021-12-23
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#include "../stdafx.hpp"

#include "../utils/map.h"

namespace job {
class QueueableJob {
    friend class QueueWorker;

  protected:
    size_t tries{0}, maxtries{3};

    bool failed{false};

  public:
    static constexpr std::string_view jobVersionStr

#ifdef CUR_JOB_FRAMEWORK_VERSION
        {CUR_JOB_FRAMEWORK_VERSION};
#else
        {""};
#endif

    static auto getJobSystemVersion() -> std::string {
        return std::string(jobVersionStr);
    }

    static auto concatJobSystemVersion(std::string_view str) -> std::string {
        std::string result;
        std::string version = getJobSystemVersion();

        result.reserve(str.size() + version.size() + 1);

        result.append(str);
        result.append(version);
        return result;
    }

    /**
     * @brief get the type name in string format, this is not compile portable,
     * to use in different programs only one compiler should be used
     *
     * @tparam T type
     */
    template <class T>
    static auto getTypeNameByInst(const T & /*ununsed*/)
        -> std::string {
        return concatJobSystemVersion(typeid(T).name());
    }

    virtual auto dump_json() const -> Poco::JSON::Object::Ptr {
        Poco::JSON::Object::Ptr json(new Poco::JSON::Object);

        json->set("tries", tries);
        json->set("maxtries", maxtries);

        return json;
    }

    virtual auto from_json(const Poco::JSON::Object::Ptr &json) -> bool {
        if (json->has("tries")) {
            tries = json->getValue<size_t>("tries");
        }
        if (json->has("maxtries")) {
            maxtries = json->getValue<size_t>("maxtries");
        }

        return true;
    }

    auto getTries() const noexcept { return tries; }
    auto getMaxTries() const noexcept { return maxtries; }
    void setMaxTries(size_t n) noexcept { maxtries = n; }

    /**
     * @brief Should retry if error occurred
     *
     * @return true should retry
     * @return false don't retry
     */
    virtual auto retryIfError() const noexcept -> bool { return true; }

    /**
     * @brief Get the job class name
     *
     * @return std::string_view the string name
     */
    virtual auto getName() const -> std::string = 0;

    /**
     * @brief the job has failed
     *
     * @return true failed
     * @return false not failed
     */
    virtual auto hasFailed() const -> bool { return failed; }

    virtual void handle() = 0;

    QueueableJob();
    virtual ~QueueableJob();

    template <class T = Poco::JSON::Object::Ptr>
    static void json_obj_get(const Poco::JSON::Object::Ptr &json,
                             const std::string &key,
                             Poco::JSON::Object::Ptr &out) {
        if (json->has(key) && !json->isNull(key)) {
            out = json->getObject(key);
        } else {
            out.reset();
        }
    }

    template <class T = Poco::JSON::Array::Ptr>
    static void json_obj_get(const Poco::JSON::Object::Ptr &json,
                             const std::string &key,
                             Poco::JSON::Array::Ptr &out) {
        if (json->has(key) && !json->isNull(key)) {
            out = json->getArray(key);
        } else {
            out.reset();
        }
    }

    template <class T>
    static void json_obj_get(const Poco::JSON::Object::Ptr &json,
                             const std::string &key, T &out) {
        if (json->has(key) && !json->isNull(key)) {
            out = json->getValue<T>(key);
        } else {
            out = T{};
        }
    }

    template <class T>
    static void json_obj_set(Poco::JSON::Object::Ptr &json,
                             const std::string &key,
                             const Poco::SharedPtr<T> &in) {
        json->set(key, in ? Poco::Dynamic::Var(in) : Poco::Dynamic::Var());
    }

    template <class T>
    static void json_obj_set(Poco::JSON::Object::Ptr &json,
                             const std::string &key,
                             const std::optional<T> &in) {
        json->set(key,
                  in ? Poco::Dynamic::Var(in.value()) : Poco::Dynamic::Var());
    }

    template <class T>
    static void json_obj_set(Poco::JSON::Object::Ptr &json,
                             const std::string &key, const T &in) {
        json->set(key, static_cast<Poco::Dynamic::Var>(in));
    }
};

} // namespace job

#define JSON_OBJ_SET(x) job::QueueableJob::json_obj_set(json, #x, x);
#define JSON_OBJ_GET(x) job::QueueableJob::json_obj_get(json, #x, x);
#define SERIALIZE_JSON_OBJ(...) MAP(JSON_OBJ_SET, __VA_ARGS__)
#define UNSERIALIZE_JSON_OBJ(...) MAP(JSON_OBJ_GET, __VA_ARGS__)

/**
 * @brief Defines the serialization functions for the job
 *
 */
#define QUEUEABLE_SERIALIZE(...)                                               \
    auto dump_json() const->Poco::JSON::Object::Ptr override {                 \
        Poco::JSON::Object::Ptr json(QueueableJob::dump_json());               \
        SERIALIZE_JSON_OBJ(__VA_ARGS__)                                        \
        return json;                                                           \
    }                                                                          \
    auto from_json(const Poco::JSON::Object::Ptr &json)->bool override {       \
        QueueableJob::from_json(json);                                         \
        UNSERIALIZE_JSON_OBJ(__VA_ARGS__)                                      \
        return true;                                                           \
    }
