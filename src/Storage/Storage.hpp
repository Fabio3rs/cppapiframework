#pragma once

#include "../stdafx.hpp"
#include <algorithm>
#include <cstdint>
#include <ios>
#include <iterator>
#include <vector>

struct Storage {
    /*
     * @brief not tested yet
     */
    static auto put(const std::filesystem::path &path,
                    const std::string &contents) -> bool {
        std::fstream out(path,
                         std::ios::binary | std::ios::out | std::ios::trunc);

        if (!out) {
            return false;
        }

        out.write(contents.c_str(),
                  static_cast<std::streamsize>(contents.size()));

        out.flush();

        return out.good();
    }

    /*
     * @brief not tested yet
     */
    static auto put(const std::filesystem::path &path,
                    const std::vector<uint8_t> &contents) -> bool {
        std::fstream out(path,
                         std::ios::binary | std::ios::out | std::ios::trunc);

        if (!out) {
            return false;
        }

        std::ostream_iterator<uint8_t> itwrite(out);
        std::copy(contents.begin(), contents.end(), itwrite);

        out.flush();

        return out.good();
    }
};
