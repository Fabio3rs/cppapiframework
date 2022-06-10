/**
 *@file CConfig.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief Configuration with environment variables
 * @version 0.1
 *
 *
 */
#pragma once
#ifndef CConfig_hpp
#define CConfig_hpp
#include <string>
#include <unordered_map>

class CConfig {
    std::unordered_map<std::string, std::string> data;

  public:
    /**
     *@brief Configuration singleton instance
     *
     * @return CConfig&
     */
    static auto config() -> CConfig &;

    /**
     *@brief Searches a variable in the config and if it is not exists return a
     *empty string
     *
     * @param key The variable name
     * @return std::string
     */
    auto operator[](const std::string &key) const noexcept -> std::string;

    /**
     *@brief Searches a variable in the config and if it is not exists return a
     *custom default value
     *
     * @param key The variable name
     * @param default_ret The default return
     * @return std::string
     */
    auto at(const std::string &key,
            std::string &&default_ret = "") const noexcept -> std::string;

    void set(const std::string &key, const std::string &value) {
        data[key] = value;
    }

    /**
     *@brief Reads and parses envp variable
     *
     * @param envp environment variables received in the third parameter of the
     *main function
     */
    void load_from_envp(const char *const *envp);

    auto operator=(const CConfig &) -> CConfig& = delete;
    CConfig(const CConfig &) = delete;

    auto operator=(CConfig &&) -> CConfig& = default;
    CConfig(CConfig &&) = default;

    ~CConfig() = default;

  private:
    CConfig() noexcept = default;
};

#endif
