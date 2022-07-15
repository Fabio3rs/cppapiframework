/**
 *@file DBMigrate.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief Cria uma tabela migrations no db e efetua migrações de comandos SQL,
 *tentando manter compatibilidade com o Laravel
 * @version 0.1
 * @date 2021-08-21
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#include <memory>
#ifndef DBMIGRATE_HPP
#define DBMIGRATE_HPP
#include "Migration.hpp"
#include <cstdint>
#include <map>
#include <string>
#include <string_view>

namespace Database {

/**
 *@brief dados de conexão com o banco de dados
 *
 */
struct DatabaseAddress {
    std::string host;
    std::string db;
    std::string user;
    std::string pwd;
};

class DBMigrate {
    std::map<std::string, std::shared_ptr<Migration>> migration_list;
    DatabaseAddress databaseInfo;

    DBMigrate() = default;

  public:
    /**
     *@brief Set the connection info object
     *
     * @param conn_info dados da conexão com o banco de dados
     */
    void set_connection_info(const DatabaseAddress &conn_info) {
        databaseInfo = conn_info;
    }

    /**
     *@brief Cria a tabela migrations se não existir
     *
     */
    void init_migration_table();

    /**
     *@brief Apaga todas as tabelas desse db
     *
     */
    void drop_all();

    /**
     *@brief Adiciona uma migração para a lista a ser migrada
     *
     * @param name nome da migração de preferência seguindo o estilo do laravel
     *(nome do arquivo de migration sem o .php)
     * @param data comando sql para migração
     */
    void push_migration(const std::string &name,
                        std::shared_ptr<Migration> data);

    /**
     *@brief Roda a lista de migrações
     *
     */
    void run();

    /**
     *@brief Objeto singleton da classe de migração
     *
     * @return DBMigrate& objeto
     */
    static auto singleton() -> DBMigrate &;

  private:
    static void migrate(const std::shared_ptr<Migration> &migration,
                        unique_conn_t &conn, std::string &query,
                        std::string &migrationName);
};
} // namespace Database

#endif
