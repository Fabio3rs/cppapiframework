#pragma once

#include "../stdafx.hpp"
#include "BorrowPool.hpp"
#include <Poco/ObjectPool.h>
#include <Poco/Redis/Client.h>
#include <Poco/Redis/Command.h>
#include <Poco/Redis/Redis.h>

class RedisService {
    using pool_t = BorrowPool<Poco::Redis::Client>;
    pool_t pool;

    std::string host, password;
    int serverport;

  public:
    auto get_connection() {
        auto borrowed = pool.borrow();
        if (!borrowed) {
            return borrowed;
        }

        connect(*borrowed);
        return borrowed;
    }

    static auto rpush(Poco::Redis::Client &inst,
                      const std::pair<std::string, std::string> &data)
        -> int64_t {
        return inst.execute<Poco::Int64>(
            Poco::Redis::Command::rpush(data.first, data.second));
    }

    template <class T>
    static inline auto eval(Poco::Redis::Client &inst,
                            std::string_view luascript,
                            const std::vector<std::string> &keys,
                            const std::vector<Poco::Dynamic::Var> &args) -> T {
        Poco::Redis::Command cmd("eval");

        cmd << luascript.data();
        cmd << std::to_string(keys.size());

        for (const auto &k : keys) {
            cmd << k;
        }

        for (const auto &a : args) {
            cmd << a.toString();
        }

        return inst.execute<T>(cmd);
    }

    template <class T>
    inline auto eval(std::string_view luascript,
                     const std::vector<std::string> &keys,
                     const std::vector<Poco::Dynamic::Var> &args) -> T {
        auto connection = get_connection();

        if (!connection) {
            return T{};
        }

        return eval<T>(*connection, luascript, keys, args);
    }

    auto rpush(const std::pair<std::string, std::string> &data) -> int64_t {
        auto connection = get_connection();

        if (!connection) {
            return -1;
        }

        return rpush(*connection, data);
    }

    static std::optional<std::pair<std::string, std::string>>
    blpop(Poco::Redis::Client &cli, const std::vector<std::string> &lista,
          int64_t timeout) {
        std::optional<std::pair<std::string, std::string>> redisElement;

        Poco::Redis::Array result = cli.execute<Poco::Redis::Array>(
            Poco::Redis::Command::blpop(lista, timeout));

        if (result.isNull()) {
            return redisElement;
        }

        redisElement = {result.get<Poco::Redis::BulkString>(0).value(),
                        result.get<Poco::Redis::BulkString>(1).value()};

        return redisElement;
    }

    std::optional<std::pair<std::string, std::string>>
    blpop(const std::vector<std::string> &lista, int64_t timeout) {
        auto connection = get_connection();

        if (!connection) {
            return std::nullopt;
        }

        return blpop(*connection, lista, timeout);
    }

    template <class T>
    static auto set_cache(Poco::Redis::Client &inst,
                          const std::pair<std::string, T> &data,
                          int key_expire = 0) {
        Poco::Redis::Command setcmd =
            Poco::Redis::Command::set(data.first, data.second, true);

        if (key_expire > 0) {
            setcmd << "EX" << std::to_string(key_expire);
        }

        return inst.execute<T>(setcmd);
    }

    template <class T>
    auto set_cache(const std::pair<std::string, T> &data, int key_expire = 0) {
        auto connection = get_connection();

        if (!connection) {
            return T{};
        }

        return set_cache<T>(*connection, data, key_expire);
    }

    template <class T>
    static auto get_cache(Poco::Redis::Client &inst,
                          const std::string &key_name) {
        return inst.execute<T>(Poco::Redis::Command::get(key_name));
    }

    template <class T> auto get_cache(const std::string &key_name) {
        auto connection = get_connection();

        if (!connection) {
            return T{};
        }

        return get_cache<T>(*connection, key_name);
    }

    auto connect(Poco::Redis::Client &inst) -> bool {
        if (inst.isConnected()) {
            return true;
        }

        try {
            inst.connect(host, serverport);

            if (!password.empty()) {
                Poco::Redis::Command authcmd("AUTH");
                authcmd << password;

                if (inst.execute<std::string>(authcmd) != "OK") {
                    throw std::invalid_argument("Senha incorreta para o REDIS");
                }
            }
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            return false;
        }

        return inst.isConnected();
    }

    static auto default_inst() -> RedisService &;

    void set_credentials(std::string h = "127.0.0.1", int port = 6379,
                         std::string pwd = std::string()) {
        host = std::move(h);
        password = std::move(pwd);
        serverport = port;
    }

    RedisService(size_t poolsize = 32, std::string h = "127.0.0.1",
                 int port = 6379, std::string pwd = std::string())
        : pool(poolsize), host(std::move(h)), password(std::move(pwd)),
          serverport(port) {}
};
