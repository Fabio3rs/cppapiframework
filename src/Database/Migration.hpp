#pragma once
#include "../stdafx.hpp"
#include "CSql.hpp"
#include <stdexcept>
#include <string>

namespace Database {
class DBMigrate;

class Migration {
    friend class DBMigrate;

  public:
    auto operator=(const Migration &) -> Migration & = delete;
    auto operator=(Migration &&) -> Migration & = default;
    Migration(const Migration &) = delete;
    Migration(Migration &&) = default;
    Migration() = default;

    virtual void migrate() = 0;
    virtual void rollback() = 0;

    virtual ~Migration();

    virtual void setConnection(sql::Connection *conn) { dbConnection = conn; }
    virtual void setQueryBuildBuffer(std::string *buffer) {
        queryBuildBuffer = buffer;
    }

    virtual void setMigrationName(const std::string &name) {
        migrationName = name;
    }

    virtual void resetTmp() {
        dbConnection = nullptr;
        queryBuildBuffer = nullptr;
        statement.reset();
    }

    virtual void begin() {
        if (dbConnection == nullptr) {
            throw std::runtime_error("No connection");
        }

        if (queryBuildBuffer == nullptr) {
            throw std::runtime_error("No query buffer");
        }

        auto &queryBuffer = *queryBuildBuffer;
        queryBuffer.clear();

        statement = unique_statement_t(dbConnection->createStatement());

        if (!statement) {
            throw std::runtime_error("No statement");
        }

        statement->execute("BEGIN;");
    }

    virtual void commit() {
        if (!statement || (dbConnection == nullptr)) {
            throw std::runtime_error("No connection or statement");
        }

        statement->execute("COMMIT;");
        dbConnection->commit();
    }

  protected:
    sql::Connection *dbConnection{nullptr};
    std::string *queryBuildBuffer{nullptr};
    std::string migrationName;
    unique_statement_t statement;
};
} // namespace Database
