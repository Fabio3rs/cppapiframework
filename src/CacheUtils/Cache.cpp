#include "Cache.hpp"
#include "../utils/RedisService.hpp"
#include "../utils/Strutils.hpp"
#include <Poco/Redis/Type.h>
#include <string>

namespace cache_utils {
std::string Cache::prefix = "apicache:";

auto Cache::getAndOrSetCache(const std::string &name,
                             const std::function<std::string()> &originalFn,
                             int key_expire) -> std::string {
    auto conn = RedisService::default_inst().get_connection();

    if (!conn) {
        return originalFn();
    }

    auto alias = Strutils::multi_concat(prefix, name);
    auto res = RedisService::get_cache<Poco::Redis::BulkString>(*conn, alias);

    if (!res.isNull()) {
        return res.value();
    }

    auto result = originalFn();

    RedisService::set<std::string>(*conn, alias, result, key_expire);

    return result;
}

} // namespace cache_utils
