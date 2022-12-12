#pragma once

#include "../stdafx.hpp"
#include <Poco/Dynamic/Var.h>
#include <Poco/Dynamic/VarHolder.h>
#include <Poco/JSON/ParseHandler.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace JSONStructParser {
template <class T, std::size_t = sizeof(
                       static_cast<void (Poco::Dynamic::VarHolder::*)(
                           T &) const>(&Poco::Dynamic::VarHolder::convert))>
std::true_type has_var_holder_overload(T *);

std::false_type has_var_holder_overload(...);

template <class T>
using has_complete_overl =
    decltype(has_var_holder_overload(std::declval<T *>()));

class StructParser;

struct StructFiller {
    using ptr_t = std::unique_ptr<StructFiller>;

    virtual void reset() = 0;
    /// Resets the handler state.

    virtual auto startObject(StructParser &parser) -> ptr_t = 0;
    /// The parser has read a {, meaning a new object will be read.

    virtual void endObject(StructParser &parser) = 0;
    /// The parser has read a }, meaning the object is read.

    virtual auto startArray(StructParser &parser) -> ptr_t = 0;
    /// The parser has read a [, meaning a new array will be read.

    virtual void endArray(StructParser &parser) = 0;
    /// The parser has read a ], meaning the array is read.

    virtual void key(StructParser &parser, const std::string &k) = 0;
    /// A key of an object is read.

    virtual void null(StructParser &parser) = 0;
    /// A null value is read.

    virtual void value(StructParser &parser, int v) = 0;
    /// An integer value is read.

    virtual void value(StructParser &parser, unsigned v) = 0;
    /// An unsigned value is read. This will only be triggered if the
    /// value cannot fit into a signed int.

    virtual void value(StructParser &parser, int64_t v) = 0;
    /// A 64-bit integer value is read.

    virtual void value(StructParser &parser, uint64_t v) = 0;

    virtual void value(StructParser &parser, const std::string &value) = 0;
    /// A string value is read.

    virtual void value(StructParser &parser, double d) = 0;
    /// A double value is read.

    virtual void value(StructParser &parser, bool b) = 0;
    /// A boolean value is read.

    virtual ~StructFiller();
};

class StructParser : public Poco::JSON::Handler {

  public:
    virtual void reset() override {}
    /// Resets the handler state.

    void startObject() override {
        if (currentKey.empty()) {
            return;
        }

        auto res = current->startObject(*this);
        stack.push(std::move(current));
        current = std::move(res);
    }
    /// Handles a '{'; a new object is started.

    void endObject() override {
        if (!stack.empty()) {
            current = std::move(stack.top());
            stack.pop();
        }
    }
    /// Handles a '}'; the object is closed.

    void startArray() override {
        if (currentKey.empty()) {
            return;
        }

        auto res = current->startArray(*this);
        stack.push(std::move(current));
        current = std::move(res);
    }
    /// Handles a '['; a new array is started.

    void endArray() override {
        if (!stack.empty()) {
            current = std::move(stack.top());
            stack.pop();
        }
    }
    /// Handles a ']'; the array is closed.

    void key(const std::string &k) override { currentKey = k; }
    /// A key is read

    Poco::Dynamic::Var asVar() const override { return {}; }
    /// Returns the result of the parser (an object or an array).

    virtual void value(int v) override {
        // std::cout  << "key " << currentKey << " : " << v << "\n";
        current->value(*this, v);
    }
    /// An integer value is read

    virtual void value(unsigned v) override {
        // std::cout  << "key " << currentKey << " : " << v << "\n";
        current->value(*this, v);
    }
    /// An unsigned value is read. This will only be triggered if the
    /// value cannot fit into a signed int.

    virtual void value(int64_t v) override {
        // std::cout  << "key " << currentKey << " : " << v << "\n";
        current->value(*this, v);
    }
    /// A 64-bit integer value is read

    virtual void value(uint64_t v) override {
        // std::cout  << "key " << currentKey << " : " << v << "\n";
        current->value(*this, v);
    }

    virtual void value(const std::string &s) override {
        // std::cout  << "key " << currentKey << " : " << s << "\n";
        current->value(*this, s);
    }
    /// A string value is read.

    virtual void value(double d) override {
        // std::cout  << "key " << currentKey << " : " << d << "\n";
        current->value(*this, d);
    }
    /// A double value is read.

    virtual void value(bool b) override {
        // std::cout  << "key " << currentKey << " : " << b << "\n";
        current->value(*this, b);
    }
    /// A boolean value is read.

    virtual void null() override {
        // std::cout  << "key " << currentKey << " : null\n";
    }
    /// A null value is read.

    std::string currentKey;
    std::unique_ptr<StructFiller> current;
    std::stack<std::unique_ptr<StructFiller>> stack;

    StructParser() = default;
    StructParser(const StructParser &) = delete;
    StructParser(StructParser &&) = delete;
    StructParser &operator=(const StructParser &) = delete;
    StructParser &operator=(StructParser &&) = delete;
    StructParser(std::unique_ptr<StructFiller> &&filler)
        : current(std::move(filler)) {}
    ~StructParser() override;
};

template <class T = void, size_t N>
inline constexpr void
ctexppairsort(std::array<std::pair<std::string_view, T>, N> &val) {
    for (size_t i = 0; i < val.size(); ++i) {
        for (size_t j = i + 1; j < val.size(); ++j) {
            if (val[i].first.compare(val[j].first) > 0) {
                auto first = std::move(val[j].first);
                auto second = std::move(val[j].second);
                val[j].first = std::move(val[i].first);
                val[j].second = std::move(val[i].second);
                val[i].first = std::move(first);
                val[i].second = std::move(second);
            }
        }
    }
}

template <class T = void, size_t N>
constexpr auto
compileTimeSort(std::array<std::pair<std::string_view, T>, N> val) {
    ctexppairsort(val);
    return val;
}

struct InitializeCallbacks {
    void (*set)(void *indata, Poco::Dynamic::VarHolder &val){};
    std::unique_ptr<StructFiller> (*makeArray)(void *indata){};
    std::unique_ptr<StructFiller> (*makeObject)(void *indata){};
};

using npair_t = std::pair<std::string_view, InitializeCallbacks>;
using namespace std::string_view_literals;

template <class T> struct startObjectFor {
    auto operator()(T & /*ref*/) -> StructFiller::ptr_t {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
};

template <class T> struct startArrayFor {
    auto operator()(T & /*ref*/) -> StructFiller::ptr_t {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
};

template <class T> struct Filter : public StructFiller {
    void reset() override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// Resets the handler state.

    auto startObject(StructParser & /*parser*/) -> ptr_t override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// The parser has read a {, meaning a new object will be read.

    void endObject(StructParser & /*parser*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// The parser has read a }, meaning the object is read.

    auto startArray(StructParser & /*parser*/) -> ptr_t override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// The parser has read a [, meaning a new array will be read.

    void endArray(StructParser & /*parser*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// The parser has read a ], meaning the array is read.

    void key(StructParser & /*parser*/, const std::string & /*k*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// A key of an object is read.

    void null(StructParser & /*parser*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// A null value is read.

    void value(StructParser & /*parser*/, int /*v*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// An integer value is read.

    void value(StructParser & /*parser*/, unsigned /*v*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// An unsigned value is read. This will only be triggered if the
    /// value cannot fit into a signed int.

    void value(StructParser & /*parser*/, int64_t /*v*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// A 64-bit integer value is read.

    void value(StructParser & /*parser*/, uint64_t /*v*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }

    void value(StructParser & /*parser*/,
               const std::string & /*value*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// A string value is read.

    void value(StructParser & /*parser*/, double /*d*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    /// A double value is read.

    void value(StructParser & /*parser*/, bool /*b*/) override {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }

    Filter() = default;
    Filter(T & /*data*/) {
        throw std::runtime_error("Unknown type " +
                                 std::string(typeid(T).name()));
    }
    virtual ~Filter() override = default;
};

template <> struct Filter<std::string> : public StructFiller {
    void reset() override { ptrdata->clear(); }
    /// Resets the handler state.

    auto startObject(StructParser & /*parser*/) -> ptr_t override {
        throw std::runtime_error("inconvesible");
    }
    /// The parser has read a {, meaning a new object will be read.

    void endObject(StructParser & /*parser*/) override {
        throw std::runtime_error("inconvesible");
    }
    /// The parser has read a }, meaning the object is read.

    auto startArray(StructParser & /*parser*/) -> ptr_t override {
        throw std::runtime_error("inconvesible");
    }
    /// The parser has read a [, meaning a new array will be read.

    void endArray(StructParser & /*parser*/) override {
        throw std::runtime_error("inconvesible");
    }
    /// The parser has read a ], meaning the array is read.

    void key(StructParser & /*parser*/, const std::string & /*k*/) override {
        throw std::runtime_error("inconvesible");
    }
    /// A key of an object is read.

    void null(StructParser & /*parser*/) override {}
    /// A null value is read.

    void value(StructParser & /*parser*/, int v) override {
        *ptrdata = std::to_string(v);
    }
    /// An integer value is read.

    void value(StructParser & /*parser*/, unsigned v) override {
        *ptrdata = std::to_string(v);
    }
    /// An unsigned value is read. This will only be triggered if the
    /// value cannot fit into a signed int.

    void value(StructParser & /*parser*/, int64_t v) override {
        *ptrdata = std::to_string(v);
    }
    /// A 64-bit integer value is read.

    void value(StructParser & /*parser*/, uint64_t v) override {
        *ptrdata = std::to_string(v);
    }

    void value(StructParser & /*parser*/, const std::string &value) override {
        *ptrdata = value;
    }
    /// A string value is read.

    void value(StructParser & /*parser*/, double d) override {
        *ptrdata = std::to_string(d);
    }
    /// A double value is read.

    void value(StructParser & /*parser*/, bool b) override {
        *ptrdata = std::to_string(b);
    }

    std::string *ptrdata{};

    Filter() = default;
    Filter(std::string &data) : ptrdata(&data) {}
    virtual ~Filter() override;
};

template <class T> struct setField {
    void operator()(T &field, Poco::Dynamic::VarHolder &val) {
        if (val.type() == typeid(T)) {
            auto &pHolderImpl =
                static_cast<Poco::Dynamic::VarHolderImpl<T> &>(val);
            field = std::move(pHolderImpl.value());
        } else {
            val.convert(field);
        }
    }
};

template <class Tp> struct setField<std::vector<Tp>> {
    void operator()(std::vector<Tp> & /*field*/,
                    Poco::Dynamic::VarHolder & /*val*/) {
        throw std::runtime_error("Unavailable");
    }
};

template <class T> auto realMakeObjectFor(T &field) {
    startObjectFor<T> maker;
    return maker(field);
}

template <class ndata_t>
inline void emplace_data(std::vector<ndata_t> &vec,
                         Poco::Dynamic::VarHolder &val, std::true_type /**/) {
    if (val.type() == typeid(ndata_t)) {
        auto &pHolderImpl =
            static_cast<Poco::Dynamic::VarHolderImpl<ndata_t> &>(val);
        vec.emplace_back(std::move(pHolderImpl.value()));
    } else {
        vec.emplace_back();
        val.convert(vec.back());
    }
}

template <class ndata_t>
inline void emplace_data(std::vector<ndata_t> &vec,
                         Poco::Dynamic::VarHolder &val, std::false_type /**/) {
    vec.emplace_back();
    setField<ndata_t> data;
    data(vec.back(), val);
}

template <class vecdata_t>
struct Filter<std::vector<vecdata_t>> : public StructFiller {
    void reset() override { ptrdata->clear(); }
    /// Resets the handler state.

    template <class T> void set(const T &data) {
        Poco::Dynamic::VarHolderImpl<T> val(data);
        emplace_data(*ptrdata, val, has_complete_overl<vecdata_t>{});
    }

    auto startObject(StructParser & /*parser*/) -> ptr_t override {
        ptrdata->emplace_back();
        startObjectFor<vecdata_t> maker;
        return maker(ptrdata->back());
    }
    /// The parser has read a {, meaning a new object will be read.

    void endObject(StructParser & /*parser*/) override {
        throw std::runtime_error("inconvesible");
    }
    /// The parser has read a }, meaning the object is read.

    auto startArray(StructParser & /*parser*/) -> ptr_t override {
        ptrdata->emplace_back();
        startArrayFor<vecdata_t> maker;
        return maker(ptrdata->back());
    }
    /// The parser has read a [, meaning a new array will be read.

    void endArray(StructParser & /*parser*/) override {
        throw std::runtime_error("inconvesible");
    }
    /// The parser has read a ], meaning the array is read.

    void key(StructParser & /*parser*/, const std::string & /*k*/) override {
        throw std::runtime_error("inconvesible");
    }
    /// A key of an object is read.

    void null(StructParser & /*parser*/) override { ptrdata->emplace_back(); }
    /// A null value is read.

    void value(StructParser & /*parser*/, int v) override { set(v); }
    /// An integer value is read.

    void value(StructParser & /*parser*/, unsigned v) override { set(v); }
    /// An unsigned value is read. This will only be triggered if the
    /// value cannot fit into a signed int.

    void value(StructParser & /*parser*/, int64_t v) override { set(v); }
    /// A 64-bit integer value is read.

    void value(StructParser & /*parser*/, uint64_t v) override { set(v); }

    void value(StructParser & /*parser*/, const std::string &value) override {
        set(value);
    }
    /// A string value is read.

    void value(StructParser & /*parser*/, double d) override { set(d); }
    /// A double value is read.

    void value(StructParser & /*parser*/, bool b) override { set(b); }

    std::vector<vecdata_t> *ptrdata{};

    Filter() = default;
    Filter(std::vector<vecdata_t> &data) : ptrdata(&data) {}
    virtual ~Filter() override = default;
};

template <class ndata_t>
inline auto move_data(Poco::Dynamic::VarHolder &val, std::true_type /**/)
    -> ndata_t {
    if (val.type() == typeid(ndata_t)) {
        auto &pHolderImpl =
            static_cast<Poco::Dynamic::VarHolderImpl<ndata_t> &>(val);
        return std::move(pHolderImpl.value());
    }

    ndata_t result;
    val.convert(result);
    return result;
}

template <class ndata_t>
inline auto move_data(Poco::Dynamic::VarHolder &val, std::false_type /**/)
    -> ndata_t {
    setField<ndata_t> data;
    ndata_t result;
    data(result, val);

    return result;
}

template <class Key, class Tp>
struct Filter<std::unordered_map<Key, Tp>> : public StructFiller {
    void reset() override { ptrdata->clear(); }
    /// Resets the handler state.

    template <class T> void set(StructParser &parser, const T &data) {
        Poco::Dynamic::VarHolderImpl<T> val(data);
        ptrdata->emplace(std::make_pair(
            parser.currentKey, move_data<Tp>(val, has_complete_overl<Tp>{})));
    }

    auto startObject(StructParser &parser) -> ptr_t override {
        auto data = ptrdata->emplace(std::pair<Key, Tp>{parser.currentKey, {}});
        startObjectFor<Tp> maker;
        return maker(data.first->second);
    }
    /// The parser has read a {, meaning a new object will be read.

    void endObject(StructParser & /*parser*/) override {
        throw std::runtime_error("inconvesible");
    }
    /// The parser has read a }, meaning the object is read.

    auto startArray(StructParser &parser) -> ptr_t override {
        auto data = ptrdata->emplace(std::pair<Key, Tp>{parser.currentKey, {}});
        startArrayFor<Tp> maker;
        return maker(data.first->second);
    }
    /// The parser has read a [, meaning a new array will be read.

    void endArray(StructParser & /*parser*/) override {
        throw std::runtime_error("inconvesible");
    }
    /// The parser has read a ], meaning the array is read.

    void key(StructParser & /*parser*/, const std::string & /*k*/) override {
        throw std::runtime_error("inconvesible");
    }
    /// A key of an object is read.

    void null(StructParser &parser) override {
        ptrdata->emplace(std::pair<Key, Tp>(parser.currentKey, {}));
    }
    /// A null value is read.

    void value(StructParser &parser, int v) override { set(parser, v); }
    /// An integer value is read.

    void value(StructParser &parser, unsigned v) override { set(parser, v); }
    /// An unsigned value is read. This will only be triggered if the
    /// value cannot fit into a signed int.

    void value(StructParser &parser, int64_t v) override { set(parser, v); }
    /// A 64-bit integer value is read.

    void value(StructParser &parser, uint64_t v) override { set(parser, v); }

    void value(StructParser &parser, const std::string &value) override {
        set(parser, value);
    }
    /// A string value is read.

    void value(StructParser &parser, double d) override { set(parser, d); }
    /// A double value is read.

    void value(StructParser &parser, bool b) override { set(parser, b); }

    std::unordered_map<Key, Tp> *ptrdata{};

    Filter() = default;
    Filter(std::unordered_map<Key, Tp> &data) : ptrdata(&data) {}
    virtual ~Filter() override = default;
};

template <class Key, class Tp> struct setField<std::unordered_map<Key, Tp>> {
    void operator()(std::unordered_map<Key, Tp> & /*field*/,
                    Poco::Dynamic::VarHolder & /*val*/) {
        throw std::runtime_error("Unavailable");
    }
};

template <class Key, class Tp>
struct startObjectFor<std::unordered_map<Key, Tp>> {
    auto operator()(std::unordered_map<Key, Tp> &ref) -> StructFiller::ptr_t {
        return std::make_unique<Filter<std::unordered_map<Key, Tp>>>(ref);
    }
};

template <class vec_t> struct startArrayFor<std::vector<vec_t>> {
    auto operator()(std::vector<vec_t> &ref) -> StructFiller::ptr_t {
        return std::make_unique<Filter<std::vector<vec_t>>>(ref);
    }
};

template <class T> struct FieldList {};

template <class T> struct TemplateStructFiller : public StructFiller {
    using struct_type = T;
    static inline auto find(const std::string &k)
        -> const InitializeCallbacks * {
        return FieldList<struct_type>::find(k);
    }

    template <class Holder_t>
    void setHolder(StructParser & /*parser*/, const std::string &k,
                   Poco::Dynamic::VarHolderImpl<Holder_t> &&val) {
        auto *res = find(k);

        if (!res) {
            throw std::runtime_error("invalid key " + k);
        }

        if (!res->set) {
            throw std::runtime_error("invalid key " + k);
        }

        res->set(ptrdata, val);
    }

    virtual void reset() override {}
    /// Resets the handler state.

    virtual auto startObject(StructParser &parser) -> ptr_t override {
        auto *res = find(parser.currentKey);

        if (!res) {
            throw std::runtime_error("invalid key " + parser.currentKey);
        }

        if (!res->makeObject) {
            throw std::runtime_error("invalid key for object");
        }

        return res->makeObject(ptrdata);
    }
    /// The parser has read a {, meaning a new object will be read.

    virtual void endObject(StructParser & /*parser*/) override {}
    /// The parser has read a }, meaning the object is read.

    virtual auto startArray(StructParser &parser) -> ptr_t override {
        auto *res = find(parser.currentKey);

        if (!res) {
            throw std::runtime_error("invalid key " + parser.currentKey);
        }

        if (!res->makeArray) {
            throw std::runtime_error("invalid key for array");
        }

        return res->makeArray(ptrdata);
    }
    /// The parser has read a [, meaning a new array will be read.

    virtual void endArray(StructParser & /*parser*/) override {}
    /// The parser has read a ], meaning the array is read.

    virtual void key(StructParser & /*parser*/,
                     const std::string & /*k*/) override {}
    /// A key of an object is read.

    virtual void null(StructParser & /*parser*/) override {}
    /// A null value is read.

    virtual void value(StructParser &parser, int v) override {
        setHolder<decltype(v)>(parser, parser.currentKey, v);
    }
    /// An integer value is read.

    virtual void value(StructParser &parser, unsigned v) override {
        setHolder<decltype(v)>(parser, parser.currentKey, v);
    }
    /// An unsigned value is read. This will only be triggered if the
    /// value cannot fit into a signed int.

    virtual void value(StructParser &parser, int64_t v) override {
        setHolder<decltype(v)>(parser, parser.currentKey, v);
    }
    /// A 64-bit integer value is read.

    virtual void value(StructParser &parser, uint64_t v) override {
        setHolder<decltype(v)>(parser, parser.currentKey, v);
    }

    virtual void value(StructParser &parser,
                       const std::string &value) override {
        setHolder<std::string>(parser, parser.currentKey, value);
    }
    /// A string value is read.

    virtual void value(StructParser &parser, double d) override {
        setHolder<decltype(d)>(parser, parser.currentKey, d);
    }
    /// A double value is read.

    virtual void value(StructParser &parser, bool b) override {
        setHolder<decltype(b)>(parser, parser.currentKey, b);
    }

    struct_type *ptrdata{};

    TemplateStructFiller() = default;
    TemplateStructFiller(struct_type &data) : ptrdata(&data) {}
    virtual ~TemplateStructFiller() override = default;
};

} // namespace JSONStructParser
