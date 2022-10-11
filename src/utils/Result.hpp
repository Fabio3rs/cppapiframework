#pragma once
/*
 * @brief Result similar to Rust
 */
#include "../stdafx.hpp"
#include <functional>

namespace utils {
enum State { OkState, ErrorState };

template <class T> struct Ok : public T {
    auto get() -> T & { return *this; }

    auto get() const -> const T & { return *this; }

    template <class fnrtn_t> auto or_else(const fnrtn_t & /**/) -> auto & {
        return *this;
    }

    template <class fnrtn_t>
    auto or_else(const fnrtn_t & /**/) const -> const auto & {
        return *this;
    }
};

template <class T> struct Err : public T {
    auto get() -> T & { return *this; }

    auto get() const -> const T & { return *this; }

    template <class fnrtn_t>
    auto or_else(const std::function<fnrtn_t()> &fun) const {
        return fun();
    }
};

template <class OkType_t, class ErrorType_t>
class Result : public std::variant<Ok<OkType_t>, Err<ErrorType_t>> {

  public:
    using error_t = Err<ErrorType_t>;
    using ok_t = Ok<OkType_t>;

    Result() = default;

    // NOLINTNEXTLINE
    Result(error_t &&errVal)
        : std::variant<ok_t, error_t>(std::forward<error_t>(errVal)) {}

    // NOLINTNEXTLINE
    Result(ok_t &&okVal)
        : std::variant<ok_t, error_t>(std::forward<ok_t>(okVal)) {}

    // NOLINTNEXTLINE
    Result(const error_t &errVal) : std::variant<ok_t, error_t>(errVal) {}

    // NOLINTNEXTLINE
    Result(const ok_t &okVal) : std::variant<ok_t, error_t>(okVal) {}

    [[nodiscard]] explicit operator bool() const {
        return std::holds_alternative<ok_t>(*this);
    }

    [[nodiscard]] auto state() const -> State {
        return std::holds_alternative<ok_t>(*this) ? State::OkState
                                                   : State::ErrorState;
    }

    auto or_else(const std::function<OkType_t()> &fun) {
        if (state() == ErrorState) {
            return fun();
        }

        return unwrap();
    }

    auto operator*() -> auto & { return std::get<0>(*this); }
    auto operator*() const -> const auto & { return std::get<0>(*this); }

    auto operator->() -> auto & { return std::get<0>(*this); }
    auto operator->() const -> const auto & { return std::get<0>(*this); }

    auto unwrap_error() -> auto & { return std::get<1>(*this); }
    auto unwrap_error() const -> const auto & { return std::get<1>(*this); }

    auto unwrap() -> OkType_t & { return std::get<0>(*this); }
    auto unwrap() const -> const OkType_t & { return std::get<0>(*this); }

    auto unwrap_or(ok_t &&defaultVal) -> auto & {
        return state() == OkState ? unwrap()
                                  : std::forward<OkType_t>(defaultVal);
    }

    auto variant() -> std::variant<ok_t, error_t> & { return *this; }

    auto variant() const -> const std::variant<ok_t, error_t> & {
        return *this;
    }
};

} // namespace utils
