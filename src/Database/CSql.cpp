/**
 *@file CSql.cpp
 * @author Fabio Rossini Sluzala ()
 * @brief CSql sql helper definitions
 * @version 0.1
 * @date 2021-05-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "CSql.hpp"
#include <iomanip>

auto CSql::instance() -> CSql & {
    static CSql sql;
    return sql;
}

auto CSql::string_to_system_clock(const std::string &str)
    -> std::optional<std::chrono::system_clock::time_point> {

    if (str.empty()) {
        return std::nullopt;
    }

    std::tm tm = {};
    std::stringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

auto CSql::make_shr_connection_cfg() -> shared_conn_t {
    auto &conf = CConfig::config();

    // std::lock_guard<std::mutex> lck(sqldrvmtx);
    sql::mysql::MySQL_Driver *driver = get_sql_drv();

    std::shared_ptr<sql::Connection> con;

    con = std::shared_ptr<sql::Connection>(driver->connect(
        conf["MYSQL_HOST"], conf["MYSQL_USER"], conf["MYSQL_PASSWORD"]));
    con->setSchema(conf["MYSQL_DATABASE"]);

    return std::reinterpret_pointer_cast<GenericDBConnection>(con);
}

auto CSql::make_connection_cfg() -> unique_conn_t {
    auto &conf = CConfig::config();
    sql::mysql::MySQL_Driver *driver = get_sql_drv();

    unique_conn_t con;

    con = unique_conn_t(driver->connect(conf["MYSQL_HOST"], conf["MYSQL_USER"],
                                        conf["MYSQL_PASSWORD"]));
    con->setSchema(conf["MYSQL_DATABASE"]);

    return con;
}

auto CSql::get_sql_drv() -> sql::mysql::MySQL_Driver * {
    /* get_driver_instance() is not thread safe */
    std::lock_guard<std::mutex> lck(sqldrvmtx);
    return sql::mysql::get_mysql_driver_instance();
}
