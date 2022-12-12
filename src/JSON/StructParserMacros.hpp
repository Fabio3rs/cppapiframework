#pragma once
#include "../utils/map.h"
#include "StructParser.hpp"

#define MAKE_START_OBJECT_SPECIALIZATION(STRUCT_T)                             \
    template <> struct startObjectFor<STRUCT_T> {                              \
        auto operator()(STRUCT_T &ref) -> StructFiller::ptr_t {                \
            return std::make_unique<TemplateStructFiller<STRUCT_T>>(ref);      \
        }                                                                      \
    };

#define MAKE_SET_FN(STRUCT_T, FIELDNAME)                                       \
    [](void *indata, Poco::Dynamic::VarHolder &val) {                          \
        auto *data = reinterpret_cast<STRUCT_T *>(indata);                     \
        setField<decltype(data->FIELDNAME)> setter;                            \
        setter(data->FIELDNAME, val);                                          \
    }

#define MAKE_ARRAY_FN(STRUCT_T, FIELDNAME)                                     \
    [](void *indata) -> std::unique_ptr<StructFiller> {                        \
        auto *data = reinterpret_cast<STRUCT_T *>(indata);                     \
        /*;*/                                                                  \
        startArrayFor<decltype(data->FIELDNAME)> maker;                        \
        return maker(data->FIELDNAME);                                         \
    }

#define MAKE_OBJECT_FN(STRUCT_T, FIELDNAME)                                    \
    [](void *indata) -> std::unique_ptr<StructFiller> {                        \
        auto *data = reinterpret_cast<STRUCT_T *>(indata);                     \
        /*;*/                                                                  \
        return realMakeObjectFor(data->FIELDNAME);                             \
    }

#define MAKE_FIELD_PAIR_STRUCT(FIELD)                                          \
    npair_t{std::string_view(#FIELD),                                          \
            {MAKE_SET_FN(struct_type, FIELD),                                  \
             MAKE_ARRAY_FN(struct_type, FIELD),                                \
             MAKE_OBJECT_FN(struct_type, FIELD)}},

#define MAKE_CUST_ARRAY(...)                                                   \
    std::array { MAP(MAKE_FIELD_PAIR_STRUCT, __VA_ARGS__) }

#define MAKE_FIELD_LIST_JS(STRUCT_T, ...)                                      \
    template <> struct FieldList<STRUCT_T> {                                   \
        using struct_type = STRUCT_T;                                          \
        static constexpr inline auto JsonInput =                               \
            compileTimeSort(MAKE_CUST_ARRAY(__VA_ARGS__));                     \
                                                                               \
        static inline auto find(const std::string &k)                          \
            -> const InitializeCallbacks * {                                   \
            auto size = JsonInput.size();                                      \
            auto i = 0U;                                                       \
                                                                               \
            for (; i < size; ++i) {                                            \
                std::cout << "comparison " << JsonInput[i].first << std::endl; \
                auto comparison = JsonInput[i].first.compare(k);               \
                if (comparison == 0) {                                         \
                    return std::addressof(JsonInput[i].second);                \
                }                                                              \
            }                                                                  \
                                                                               \
            return nullptr;                                                    \
        }                                                                      \
    }

#define MAKE_DISABLE_SET_STRUCT(STRUCT_T)                                      \
    template <> struct setField<STRUCT_T> {                                    \
        [[noreturn]] void operator()(STRUCT_T & /*field*/,                     \
                                     Poco::Dynamic::VarHolder & /*val*/) {     \
            throw std::runtime_error("Unavailable for " #STRUCT_T);            \
        }                                                                      \
    }
