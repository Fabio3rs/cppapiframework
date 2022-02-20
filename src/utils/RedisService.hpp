#pragma once

#include "../stdafx.hpp"
#include "BorrowPool.hpp"
#include <Poco/ObjectPool.h>
#include <Poco/Redis/Client.h>
#include <Poco/Redis/Command.h>
#include <Poco/Redis/Redis.h>

struct RedisServiceAddress {
    std::string host;
    int serverport{0};
};

class RedisService {
    using pool_t = BorrowPool<Poco::Redis::Client>;
    pool_t pool;

    std::vector<RedisServiceAddress> replicaList;

    std::string password;

  public:
    auto get_connection() {
        auto borrowed = pool.borrow();
        if (!borrowed) {
            return borrowed;
        }

        connect(*borrowed, borrowed.getId());
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

    static auto blpop(Poco::Redis::Client &cli,
                      const std::vector<std::string> &lista, int64_t timeout)
        -> std::optional<std::pair<std::string, std::string>>;

    auto blpop(const std::vector<std::string> &lista, int64_t timeout)
        -> std::optional<std::pair<std::string, std::string>>;

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

    template <class T>
    static auto hget(Poco::Redis::Client &inst, const std::string &key_name,
                     const std::string &field) {
        return inst.execute<T>(Poco::Redis::Command::hget(key_name, field));
    }

    template <class T>
    auto hget(const std::string &key_name, const std::string &field) {
        auto connection = get_connection();

        if (!connection) {
            return T{};
        }

        return hget<T>(*connection, key_name, field);
    }

    static auto hgetall(Poco::Redis::Client &inst,
                        const std::string &key_name) {
        auto result = inst.execute<Poco::Redis::Array>(
            Poco::Redis::Command::hgetall(key_name));

        std::unordered_map<std::string, std::string> resultmap;
        if (result.isNull()) {
            return resultmap;
        }

        if (result.size() % 2 != 0) {
            throw std::runtime_error("hgetall result is not multiple of 2");
        }

        resultmap.reserve(result.size() / 2);

        for (unsigned int i = 0; i < result.size(); i += 2) {
            resultmap[result.get<Poco::Redis::BulkString>(i).value(
                std::string())] =
                result.get<Poco::Redis::BulkString>(i + 1).value(std::string());
        }

        return resultmap;
    }

    auto hgetall(const std::string &key_name)
        -> std::unordered_map<std::string, std::string> {
        auto connection = get_connection();

        if (!connection) {
            return {};
        }

        return hgetall(*connection, key_name);
    }

    /**
     * @brief hset is like a map in the redis
     *
     * @param inst redis client instance
     * @param keyname name of the key
     * @param map map of the values
     * @return auto int64_t number of the new elements
     */
    static auto hset(Poco::Redis::Client &inst, const std::string &keyname,
                     const std::unordered_map<std::string, std::string> &map)
        -> int64_t {
        Poco::Redis::Command hsetcmd = Poco::Redis::Command("hset");

        hsetcmd << keyname;

        for (const auto &keypair : map) {
            hsetcmd << keypair.first << keypair.second;
        }

        return inst.execute<int64_t>(hsetcmd);
    }

    auto hset(const std::string &key_name,
              const std::unordered_map<std::string, std::string> &map)
        -> int64_t {
        auto connection = get_connection();

        if (!connection) {
            return -1;
        }

        return hset(*connection, key_name, map);
    }

    static auto hdel(Poco::Redis::Client &inst, const std::string &key_name,
                     const std::vector<std::string> &fieldsname) -> int64_t {
        Poco::Redis::Command hdelcmd =
            Poco::Redis::Command::hdel(key_name, fieldsname);
        return inst.execute<int64_t>(hdelcmd);
    }

    auto hdel(const std::string &key_name,
              const std::vector<std::string> &fieldsname) -> int64_t {
        auto connection = get_connection();

        if (!connection) {
            return -1;
        }

        return hdel(*connection, key_name, fieldsname);
    }

    static auto del(Poco::Redis::Client &inst,
                    const std::vector<std::string> &keysname) -> int64_t {
        Poco::Redis::Command delcmd = Poco::Redis::Command::del(keysname);
        return inst.execute<int64_t>(delcmd);
    }

    auto del(const std::vector<std::string> &keysname) -> int64_t {
        auto connection = get_connection();

        if (!connection) {
            return -1;
        }

        return del(*connection, keysname);
    }

    class argToString {
        const std::string str;

      public:
        [[nodiscard]] inline auto getStr() const -> const std::string & {
            return str;
        }

        // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
        inline argToString(bool value) : str(value ? "true" : "false") {}

        // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
        inline argToString(const char *s) : str(s) {}

        // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
        inline argToString(const std::exception &e) : str(e.what()) {}

        // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
        inline argToString(std::string s) : str(std::move(s)) {}

        template <class T,
                  typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
        inline argToString(T value)
            : str(std::to_string(value)) {
        }
    };

    template <class T, class... Types>
    static inline auto cmd_inst(Poco::Redis::Client &inst,
                                const std::string &cmd, Types &&... args) -> T {
        Poco::Redis::Command custom_cmd = Poco::Redis::Command(cmd);

        std::array<argToString, std::tuple_size<std::tuple<Types...>>::value>
            argslist = {std::forward<Types>(args)...};

        for (auto &a : argslist) {
            custom_cmd.add(a.getStr());
        }

        return inst.execute<T>(custom_cmd);
    }

    template <class T, class... Types>
    inline auto cmd(const std::string &cmd, Types &&... args) -> T {
        auto connection = get_connection();

        if (!connection) {
            return T{};
        }

        return cmd_inst<T>(*connection, cmd, std::forward<Types>(args)...);
    }

    auto connect(Poco::Redis::Client &inst, size_t chooseReplica = 0) -> bool {
        if (inst.isConnected()) {
            return true;
        }

        try {
            const auto &choosenHost =
                replicaList[chooseReplica % replicaList.size()];
            inst.connect(choosenHost.host, choosenHost.serverport);

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
        if (replicaList.empty()) {
            replicaList.push_back({std::move(h), port});
        } else {
            RedisServiceAddress &raddr = replicaList[0];
            raddr.host = std::move(h);
            raddr.serverport = port;
        }

        password = std::move(pwd);
    }

    void add_host_replica(std::string h = "127.0.0.1", int port = 6379) {
        replicaList.push_back({std::move(h), port});
    }

    RedisService(size_t poolsize, std::vector<RedisServiceAddress> replicas,
                 std::string pwd = std::string());

    explicit RedisService(size_t poolsize = 32, std::string h = "127.0.0.1",
                          int port = 6379, std::string pwd = std::string());
};
