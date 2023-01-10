#pragma once

#include "../stdafx.hpp"
#include <algorithm>
#include <cstdint>
#include <ios>
#include <iterator>
#include <string>
#include <vector>

/*
 * @brief not tested yet
 */
struct Storage {
    static auto put(const std::filesystem::path &path,
                    const std::string &contents) -> bool;

    static auto put(const std::filesystem::path &path,
                    const std::vector<uint8_t> &contents) -> bool;

    static inline auto publicPath() -> std::filesystem::path {
        return "./storage/public";
    }

    static inline auto buildPublicPath(const std::string &name) {
        return publicPath() / name;
    }

    static auto putPublicly(const std::string &name,
                            const std::string &contents)
        -> std::filesystem::path;

    static auto putPublicly(const std::string &name,
                            const std::vector<uint8_t> &contents)
        -> std::filesystem::path;

    static auto uriFromPublic(const std::filesystem::path &path) -> std::string;
};
