#pragma once

#include <cstdint>
#include <unordered_map>

struct primitivepairhash {
  public:
    template <typename T, typename U>
    auto operator()(const std::pair<T, U> &pair) const -> std::size_t {
        return std::hash<T>()(pair.first) ^ std::hash<U>()(pair.second);
    }
};
