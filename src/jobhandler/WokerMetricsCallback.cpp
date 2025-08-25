#include "WorkerMetricsCallback.hpp"

// Define ~WorkerMetricsCallback()
// to avoid weak vtables warning
// when the destructor is not inline
namespace job {
WorkerMetricsCallback::~WorkerMetricsCallback() = default;

} // namespace job
