#include "DBMigrate.hpp"
#include "../stdafx.hpp"
#include <cppconn/prepared_statement.h>
#include <memory>
#include <utility>

namespace Database {
static auto connect_db(const DatabaseAddress &info) {
    sql::mysql::MySQL_Driver *driver = CSql::instance().get_sql_drv();

    unique_conn_t con;

    con = unique_conn_t(driver->connect(info.host, info.user, info.pwd));
    con->setSchema(info.db);

    return con;
}

auto DBMigrate::singleton() -> DBMigrate & {
    static DBMigrate instance;
    return instance;
}

void DBMigrate::init_migration_table() {
    auto conn = connect_db(databaseInfo);
    std::string query =
        "CREATE TABLE IF NOT EXISTS `migrations` (   `id` int(10) unsigned NOT "
        "NULL "
        "AUTO_INCREMENT,   `migration` varchar(255) COLLATE utf8mb4_unicode_ci "
        "NOT "
        "NULL,   `batch` int(11) NOT NULL,   PRIMARY KEY (`id`) ) "
        "ENGINE=InnoDB "
        "AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;";

    auto ssmt = unique_statement_t(conn->createStatement());

    ssmt->execute(query);
}

void DBMigrate::drop_all() {
    /**
     *@brief Unimplemented
     *
     */
}

void DBMigrate::push_migration(const std::string &name,
                               std::shared_ptr<Migration> data) {
    migration_list[name] = std::move(data);
}

static void
build_insert_migration_query(std::string &query, const std::string &migration,
                             sql::mysql::MySQL_Connection *msqlconn) {
    using pfd = CSql::field_data_pair_t;
    query.clear();
    CSql::build_generic_insert_query(
        query, "migrations",
        std::array{pfd{"migration", CSql::esc_add_q(msqlconn, migration)},
                   pfd{"batch", 1}});

    query += ";";
}

void DBMigrate::migrate(const std::shared_ptr<Migration> &migration,
                        unique_conn_t &conn, std::string &query,
                        std::string &migrationName) {
    migration->setConnection(conn.get());
    migration->setQueryBuildBuffer(&query);
    migration->setMigrationName(migrationName);

    migration->begin();
    migration->migrate();

    query.clear();
    build_insert_migration_query(query, migrationName,
                                 CSql::mysql_cast(conn.get()));
    migration->statement->execute(query);
    migration->commit();

    migration->resetTmp();
}

static auto is_migrated(const std::string &name, unique_conn_t &conn) -> bool {
    std::unique_ptr<sql::PreparedStatement> stmnt(
        conn->prepareStatement("SELECT COUNT(id) as nmigration FROM migrations "
                               "WHERE migration = ? LIMIT 1"));

    stmnt->setString(1, name);
    auto res = unique_resultset_t(stmnt->executeQuery());

    if (!res->next()) {
        return false;
    }

    return res->getUInt64("nmigration") != 0;
}

void DBMigrate::run() {
    auto conn = connect_db(databaseInfo);
    conn->setAutoCommit(false);

    std::string query;
    query.reserve(512);

    for (auto &migrating : migration_list) {
        if (is_migrated(migrating.first, conn)) {
            continue;
        }

        std::string migrationName = migrating.first;
        std::cout << "\033[33mMigrating:\033[39m " << migrationName
                  << std::endl;
        auto &migration = migrating.second;

        try {
            migrate(migration, conn, query, migrationName);
            std::cout << "\033[32mMigrated:\033[39m " << migrationName
                      << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "\033[31mMigration failed:\033[39m " << migrationName
                      << ": " << e.what() << '\n';
            std::cerr << "DEBUG QUERY: " << query << std::endl;
            migration->resetTmp();
            throw;
        }
    }
}
} // namespace Database
