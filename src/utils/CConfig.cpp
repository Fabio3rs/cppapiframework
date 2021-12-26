/**
 *@file CConfig.cpp
 * @author Fabio Rossini Sluzala ()
 * @brief Config class definitions
 * @version 0.1
 *
 */
#include "CConfig.hpp"
#include <algorithm>

auto CConfig::config() -> CConfig & {
    static CConfig conf;
    return conf;
}

auto CConfig::operator[](const std::string &key) const noexcept -> std::string {
    auto it = data.find(key);
    if (it == data.end()) {
        return "";
    }

    return it->second;
}

auto CConfig::at(const std::string &key,
                 std::string &&default_ret) const noexcept -> std::string {
    auto it = data.find(key);
    if (it == data.end()) {
        return std::move(default_ret);
    }

    return it->second;
}

void CConfig::load_from_envp(const char *const *envp) {
    const char *const *env = envp;
    std::string line;
    std::string key;
    std::string val;
    for (/**/; *env != nullptr; ++env) {
        line = *env;

        size_t pos = line.find_first_of('=');

        if (pos == std::string::npos) {
            continue;
        }

        key = line.substr(0, pos);
        val = line.substr(pos + 1);

        data[key] = val;
    }
}
