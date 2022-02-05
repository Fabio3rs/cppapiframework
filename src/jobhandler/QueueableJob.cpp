#include "QueueableJob.hpp"

job::QueueableJob::QueueableJob() = default;

job::QueueableJob::~QueueableJob() = default;

auto job::QueueableJob::dump_json() const -> Poco::JSON::Object::Ptr {
    Poco::JSON::Object::Ptr json(new Poco::JSON::Object);

    json->set("tries", tries);
    json->set("maxtries", maxtries);

    return json;
}
