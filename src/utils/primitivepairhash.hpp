#pragma once

#include <cstdint>
#include <unordered_map>

struct primitivepairhash {
  public:
    template <typename T, typename U>
    std::size_t operator()(const std::pair<T, U> &pair) const {
        return std::hash<T>()(pair.first) ^ std::hash<U>()(pair.second);
    }
};
