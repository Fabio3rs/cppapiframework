#include "JobsHandler.hpp"

auto job::JobsHandler::default_instance() -> std::shared_ptr<JobsHandler> {
    static std::shared_ptr<JobsHandler> instance(
        std::make_shared<JobsHandler>());
    return instance;
}
