#pragma once

#include "../stdafx.hpp"

namespace cache_utils {

struct Cache {
    static std::string prefix;

    static auto getAndOrSetCache(const std::string &name,
                                 const std::function<std::string()> &originalFn,
                                 int key_expire = 0) -> std::string;
};

} // namespace cache_utils
