#pragma once
/**
 * @brief Execute a callback when an exception is thrown and not caught.
 */

#include <exception>
#include <functional>
#include <utility>

struct ExceptUnwindExec {
    std::function<void()> cb;

    explicit ExceptUnwindExec(std::function<void()> callback)
        : cb(std::move(callback)) {}

    ExceptUnwindExec(const ExceptUnwindExec &other) = default;

    auto operator=(const ExceptUnwindExec &other)
        -> ExceptUnwindExec & = default;

    ExceptUnwindExec(ExceptUnwindExec &&other) noexcept = default;

    auto operator=(ExceptUnwindExec &&other) noexcept
        -> ExceptUnwindExec & = default;

    ~ExceptUnwindExec() noexcept {
        if (std::uncaught_exceptions() == 0) {
            return;
        }

        try {
            cb();
        } catch (const std::exception &e) {
            std::cerr << __FILE__ << ": " << e.what() << '\n';
        } catch (...) {
            std::cerr << "Unknown exception\n";
        }
    }
};
