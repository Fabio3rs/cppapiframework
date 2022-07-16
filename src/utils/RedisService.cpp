#include "RedisService.hpp"
#include "CConfig.hpp"
#include <exception>

auto RedisService::default_inst() -> RedisService & {
    static RedisService instance;
    return instance;
}

RedisService::RedisService(size_t poolsize, std::string HostAddr, int port,
                           std::string pwd)
    : pool(poolsize) {
    auto &config = CConfig::config();
    if (HostAddr.empty()) {
        HostAddr = config.at("REDIS_HOST", "127.0.0.1");
    }

    if (port == 0) {
        try {
            port = std::stoi(config.at("REDIS_PORT", "6379"));
        } catch (const std::exception &ex) {
            std::cerr << "Invalid REDIS_PORT, error: " << ex.what()
                      << " data: " << config.at("REDIS_PORT") << std::endl;
            port = 6379;
        }
    }

    if (pwd.empty()) {
        pwd = config.at("REDIS_PASSWORD", "");
    }

    replicaList.push_back({std::move(HostAddr), port});

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
