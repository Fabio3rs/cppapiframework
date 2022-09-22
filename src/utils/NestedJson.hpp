#pragma once
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>

namespace NestedJson {
template <class T, class... Types>
auto getNestedValue(const Poco::JSON::Object::Ptr &startobject, Types &&...args)
    -> T {
    const std::array<std::string_view,
                     std::tuple_size<std::tuple<Types...>>::value>
        arglist{std::forward<std::string_view>(args)...};

    if (arglist.empty()) {
        return {};
    }

    Poco::JSON::Object::Ptr current = startobject;

    auto lastit = std::end(arglist) - 1;

    for (auto it = arglist.begin(), end = lastit; it != end; it++) {
        current = current->getObject(std::string(*it));
    }

    return current->getValue<T>(std::string(*lastit));
}

template <class C, typename = std::enable_if_t<std::is_arithmetic<C>::value>>
auto getNextNested(const Poco::Dynamic::Var &enclosing, C index)
    -> Poco::Dynamic::Var {
    using ArrPtr = Poco::JSON::Array::Ptr;
    return enclosing.extract<ArrPtr>()->get(static_cast<unsigned int>(index));
}

inline auto getNextNested(const Poco::Dynamic::Var &enclosing,
                          const std::string &name) -> Poco::Dynamic::Var {
    using ObjPtr = Poco::JSON::Object::Ptr;
    return enclosing.extract<ObjPtr>()->get(name);
}

template <class T, class... Types>
auto getArrObjNestedValue(const Poco::Dynamic::Var &enclosing, Types &&...args)
    -> T {
    const std::tuple<Types...> arglist{std::forward<Types>(args)...};

    if (std::tuple_size<std::tuple<Types...>>::value == 0) {
        return {};
    }

    Poco::Dynamic::Var last = enclosing;

    // NOLINTNEXTLINE(hicpp-no-array-decay)
    ((last = getNextNested(last, args)), ...);

    return last.convert<T>();
}

template <class... Types>
auto getArrObjNestedVar(const Poco::Dynamic::Var &enclosing, Types &&...args)
    -> Poco::Dynamic::Var {
    const std::tuple<Types...> arglist{std::forward<Types>(args)...};

    if (std::tuple_size<std::tuple<Types...>>::value == 0) {
        return {};
    }

    Poco::Dynamic::Var last = enclosing;

    // NOLINTNEXTLINE(hicpp-no-array-decay)
    ((last = getNextNested(last, args)), ...);

    return last;
}

template <class T, class... Types>
auto getArrObjNestedExtract(const Poco::Dynamic::Var &enclosing,
                            Types &&...args) -> T {
    const std::tuple<Types...> arglist{std::forward<Types>(args)...};

    if (std::tuple_size<std::tuple<Types...>>::value == 0) {
        return {};
    }

    Poco::Dynamic::Var last = enclosing;

    // NOLINTNEXTLINE(hicpp-no-array-decay)
    ((last = getNextNested(last, args)), ...);

    return last.extract<T>();
}

} // namespace NestedJson
