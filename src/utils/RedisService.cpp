#include "RedisService.hpp"

auto RedisService::default_inst() -> RedisService & {
    static RedisService instance;
    return instance;
}

RedisService::RedisService(size_t poolsize, std::string h, int port,
                           std::string pwd)
    : pool(poolsize) {
    replicaList.push_back({std::move(h), port});

    password = std::move(pwd);
}

RedisService::RedisService(size_t poolsize,
                           std::vector<RedisServiceAddress> replicas,
                           std::string pwd)
    : pool(poolsize), replicaList(std::move(replicas)) {
    password = std::move(pwd);
}

auto RedisService::blpop(Poco::Redis::Client &cli,
                         const std::vector<std::string> &lista, int64_t timeout)
    -> std::optional<std::pair<std::string, std::string>> {
    std::optional<std::pair<std::string, std::string>> redisElement;

    auto result = cli.execute<Poco::Redis::Array>(
        Poco::Redis::Command::blpop(lista, timeout));

    if (result.isNull()) {
        return redisElement;
    }

    redisElement = {result.get<Poco::Redis::BulkString>(0).value(),
                    result.get<Poco::Redis::BulkString>(1).value()};

    return redisElement;
}

auto RedisService::blpop(const std::vector<std::string> &lista, int64_t timeout)
    -> std::optional<std::pair<std::string, std::string>> {
    auto connection = get_connection();

    if (!connection) {
        return std::nullopt;
    }

    return blpop(*connection, lista, timeout);
}
