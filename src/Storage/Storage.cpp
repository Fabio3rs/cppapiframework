/*
 * @brief not tested yet
 */

#include "Storage.hpp"
#include <filesystem>

auto Storage::put(const std::filesystem::path &path,
                  const std::string &contents) -> bool {
    std::fstream out(path, std::ios::binary | std::ios::out | std::ios::trunc);

    if (!out) {
        return false;
    }

    out.write(contents.c_str(), static_cast<std::streamsize>(contents.size()));

    out.flush();

    return out.good();
}

auto Storage::put(const std::filesystem::path &path,
                  const std::vector<uint8_t> &contents) -> bool {
    std::fstream out(path, std::ios::binary | std::ios::out | std::ios::trunc);

    if (!out) {
        return false;
    }

    std::ostream_iterator<uint8_t> itwrite(out);
    std::copy(contents.begin(), contents.end(), itwrite);

    out.flush();

    return out.good();
}

auto Storage::putPublicly(const std::string &name, const std::string &contents)
    -> std::filesystem::path {
    auto resultPath = buildPublicPath(name);

    if (!put(resultPath, contents)) {
        return {};
    }

    return resultPath;
}

auto Storage::putPublicly(const std::string &name,
                          const std::vector<uint8_t> &contents)
    -> std::filesystem::path {
    auto resultPath = buildPublicPath(name);

    if (!put(resultPath, contents)) {
        return {};
    }

    return resultPath;
}

auto Storage::uriFromPublic(const std::filesystem::path &path) -> std::string {
    // TODO: dynamically path detect
    return "/storage/public/" + path.filename().string();
}
