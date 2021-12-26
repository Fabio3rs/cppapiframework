#include "RedisService.hpp"

auto RedisService::default_inst() -> RedisService & {
    static RedisService instance;
    return instance;
}
