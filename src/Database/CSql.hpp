/**
 *@file CSql.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief CSql sql helper header
 * @version 0.1
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#ifndef CSql_hpp
#define CSql_hpp

#include "../stdafx.hpp"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_connection.h>
#include <mysql_driver.h>

typedef std::unique_ptr<sql::Connection> unique_conn_t;
typedef std::shared_ptr<GenericDBConnection> shared_conn_t;
using unique_prepstatement_t = std::unique_ptr<sql::PreparedStatement>;
using unique_statement_t = std::unique_ptr<sql::Statement>;
using unique_resultset_t = std::unique_ptr<sql::ResultSet>;

class CSql {
    std::mutex sqldrvmtx;

    CSql() = default;
    CSql(const CSql &) = delete;

  public:
    static inline auto
    high_precision_time_to_str(std::chrono::high_resolution_clock::time_point t)
        -> std::string {
        std::time_t tt = std::chrono::high_resolution_clock::to_time_t(t);

        auto mksec = std::chrono::duration_cast<std::chrono::microseconds>(
                         t.time_since_epoch())
                         .count();
        mksec %= 1000000;

        std::string str;

        {
            std::array<char, 32> buf{};

            size_t strft_res_sz =
                strftime(buf.data(), buf.size(), "%Y/%m/%d %H:%M:%S.",
                         std::localtime(&tt));

            str.reserve(28);
            str.append(buf.data(), strft_res_sz);
        }

        {
            std::string mksecstr = std::to_string(mksec);
            size_t mksecsz = mksecstr.size();

            if (mksecsz < 6) {
                {
                    str.append(6 - mksecsz, '0');
                }
            }

            str += mksecstr;
        }

        return str;
    }

    static inline auto
    system_time_to_str(std::optional<std::chrono::system_clock::time_point> t)
        -> std::string {
        if (!t.has_value()) {
            return std::string();
        }

        std::time_t tt = std::chrono::system_clock::to_time_t(t.value());

        std::string str;

        {
            std::array<char, 32> buf{};

            size_t strft_res_sz =
                strftime(buf.data(), buf.size(), "%Y/%m/%d %H:%M:%S",
                         std::localtime(&tt));

            str.reserve(28);
            str.append(buf.data(), strft_res_sz);
        }

        return str;
    }

    static auto string_to_system_clock(const std::string &str)
        -> std::optional<std::chrono::system_clock::time_point>;

    auto make_shr_connection_cfg() -> shared_conn_t;

    /**
     *@brief Conversão genérica de valores de multiplos tipos para string
     *
     */
    class argToString {
        const std::string str;

      public:
        [[nodiscard]] auto getStr() const -> std::string_view { return str; }

        [[nodiscard]] auto size() const { return str.size(); }

        explicit argToString(bool value) : str(value ? "true" : "false") {}

        explicit argToString(const char *s) : str(s) {}

        explicit argToString(const std::exception &e) : str(e.what()) {}

        explicit argToString(std::string s) : str(std::move(s)) {}

        template <class T,
                  typename = std::enable_if_t<std::is_arithmetic<T>::value>>
        explicit argToString(T value) : str(std::to_string(value)) {}

        constexpr explicit operator std::string_view() { return str; }
    };

    using field_data_pair_t = std::pair<std::string_view, argToString>;

    // Singleton
    static auto instance() -> CSql &;

    /**
     *@brief Usa a classe CConfig para obter credenciais e gerar uma conexão a
     *um servidor Mysql
     *
     * @return unique_conn_t unique da conexão
     */
    auto make_connection_cfg() -> unique_conn_t;

    /**
     *@brief Get the sql drv object. Objeto de driver do sql não é garantido
     *contra concorrência
     *
     * @return sql::mysql::MySQL_Driver* ponteiro
     */
    auto get_sql_drv() -> sql::mysql::MySQL_Driver *;

    static inline auto mysql_cast(sql::Connection *conn)
        -> sql::mysql::MySQL_Connection * {
        sql::mysql::MySQL_Connection *mconn =
            dynamic_cast<sql::mysql::MySQL_Connection *>(conn);

        if (mconn == nullptr) {
            throw std::bad_cast();
        }

        return mconn;
    }

    /**
     *@brief Escape de string para evitar SQL Injection e inserir corretamente
     *strings com caracteres não textuais
     *
     * @param conn unique de conexão ao banco
     * @param val valor a ser escapado
     * @param null_if_empty retorna o texto NULL se a strng estiver vazia
     * @return std::string resultado do escape
     */
    static inline auto escape(sql::mysql::MySQL_Connection *conn,
                              const std::string &val,
                              bool null_if_empty = false) -> std::string {
        if (conn == nullptr) {
            {
                throw std::invalid_argument("Resource is null");
            }
        }

        if (null_if_empty && val.empty()) {
            return "NULL";
        }

        return conn->escapeString(val);
    }

    /**
     *@brief Performs escape of special characters in string to use in sql
     *queries
     *
     * @param conn the mysql connection pointer
     * @param val string value to escape
     * @param null_if_empty return NULL if the string is empty
     * @return std::string
     */
    static inline auto esc_add_q(sql::mysql::MySQL_Connection *conn,
                                 const std::string &val,
                                 bool null_if_empty = false) -> std::string {
        if (conn == nullptr) {
            {
                throw std::invalid_argument("Resource is null");
            }
        }

        if (null_if_empty && val.empty()) {
            return "NULL";
        }

        std::string result;
        result.reserve(val.size() + 2);

        result = "'";
        result += conn->escapeString(val);
        result += "'";

        return result;
    }

    /**
     *@brief Adiciona função UNHEX() de SQL em uma string
     *
     * @param conn conexão ao banco de dados
     * @param val string
     * @param null_if_empty retornar NULL se o val for vazio
     * @return std::string
     */
    static inline auto
    escape_add_qwith_unhex(sql::mysql::MySQL_Connection *conn,
                           const std::string &val, bool null_if_empty = false)
        -> std::string {
        if (conn == nullptr) {
            {
                throw std::invalid_argument("Resource is null");
            }
        }

        if (null_if_empty && val.empty()) {
            return "NULL";
        }

        std::string result;
        result.reserve(val.size() + 9);

        result = "UNHEX('";
        result += conn->escapeString(val);
        result += "')";

        return result;
    }

    template <class... Types, size_t N>
    static inline constexpr void build_generic_insert_query(
        std::string &fullquery, const std::string_view table_name,
        std::array<field_data_pair_t, N> const &fielddata) noexcept {
        {
            size_t fieldListSize = 0;
            size_t fieldDataSize = 0;

            for (const auto &arg : fielddata) {
                fieldListSize += arg.first.size() + 2;
                fieldDataSize += arg.second.size() + 2;
            }

            const size_t FINAL_QUERY_SIZE =
                (sizeof("INSERT INTO () VALUES ();") + table_name.size() +
                 fieldListSize + fieldDataSize); /*"(" + fieldList + ") VALUES
                                                    (" + fieldData + ")"*/

            if ((fullquery.capacity() - fullquery.size()) < FINAL_QUERY_SIZE) {
                fullquery.reserve(FINAL_QUERY_SIZE);
            }
        }

        bool firstList = true;

        {
            fullquery += "INSERT INTO ";
            fullquery += table_name;
            fullquery += "(";
            for (const auto &arg : fielddata) {
                if (!firstList) {
                    fullquery += ", ";
                }

                fullquery += arg.first;
                firstList = false;
            }
            fullquery += ") VALUES (";
        }

        {
            firstList = true;

            for (const auto &arg : fielddata) {
                if (!firstList) {
                    fullquery += ", ";
                }

                fullquery += arg.second.getStr();
                firstList = false;
            }
            fullquery += ")";
        }
    }

    static auto get_insert_query_size(std::string_view table_name,
                                      const Poco::JSON::Object::Ptr &fielddata)
        -> size_t {
        size_t fieldListSize = 0;
        size_t fieldDataSize = 0;

        for (const auto &arg : *fielddata) {
            fieldListSize += arg.first.size() + 2;
            fieldDataSize +=
                arg.second.size() + (arg.second.isString() ? 4 : 2);
        }

        const size_t FINAL_QUERY_SIZE =
            (sizeof("INSERT INTO () VALUES ();") + table_name.size() +
             fieldListSize + fieldDataSize); /*"(" + fieldList + ") VALUES (" +
                                                fieldData + ")"*/

        return FINAL_QUERY_SIZE;
    }

    static auto
    append_comma_separed_fields(std::string &fullquery,
                                const Poco::JSON::Object::Ptr &fielddata) {
        bool firstList = true;

        for (const auto &arg : *fielddata) {
            if (!firstList) {
                fullquery += ", ";
            }

            fullquery += arg.first;
            firstList = false;
        }
    }

    static auto
    append_comma_separed_values(std::string &fullquery,
                                const Poco::JSON::Object::Ptr &fielddata,
                                sql::mysql::MySQL_Connection *mscon) {
        bool firstList = true;

        for (const auto &arg : *fielddata) {
            if (!firstList) {
                fullquery += ", ";
            }

            if (arg.second.isNumeric()) {
                fullquery += arg.second.toString();
            } else {
                fullquery += esc_add_q(mscon, arg.second.toString());
            }
            firstList = false;
        }
    }

    static auto should_resize_string(size_t desired_size, std::string &str)
        -> bool {
        return (str.capacity() - str.size()) < desired_size;
    }

    static void
    build_generic_insert_query_json(std::string &fullquery,
                                    std::string_view table_name,
                                    const Poco::JSON::Object::Ptr &fielddata,
                                    sql::mysql::MySQL_Connection *mscon) {
        size_t FINAL_QUERY_SIZE = get_insert_query_size(table_name, fielddata);

        if (should_resize_string(FINAL_QUERY_SIZE, fullquery)) {
            fullquery.reserve(FINAL_QUERY_SIZE);
        }

        fullquery += "INSERT INTO ";
        fullquery += table_name;
        fullquery += "(";

        append_comma_separed_fields(fullquery, fielddata);

        fullquery += ") VALUES (";

        append_comma_separed_values(fullquery, fielddata, mscon);

        fullquery += ")";
    }

    static auto inline get_last_inserted_id(unique_statement_t &stmt)
        -> uint64_t {
        unique_resultset_t results(
            stmt->executeQuery("SELECT LAST_INSERT_ID() as id;"));

        if (!results->next()) {
            return 0;
        }

        return results->getUInt64("id");
    }
};

#endif
