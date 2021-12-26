/**
 *@file InputValidators.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief Fornece classes para validação de input de dados json
 * @todo Separar as sub-classes em arquivos próprios
 * @version 0.1
 * @date 2021-07-20
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#ifndef INPUTVALIDATORS_HPP
#define INPUTVALIDATORS_HPP
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>

#include <regex>
#include <string>
#include <string_view>
#include <utility>

/**
 *@brief Base da interface de inputs usados nos validadores de input json,
 *
 */
class InputValidator {

  public:
    /**
     *@brief Retorna uma mensagem em caso de falha de validação
     *
     * @param fieldname nome do campo
     * @param s valor do campo
     * @return std::optional<std::string> retorno, std::nullopt se estiver tudo
     *certo, std::string para mensagem de erro
     */
    virtual auto validate(std::string_view fieldname,
                          const Poco::Dynamic::Var &s)
        -> std::optional<std::string> = 0;

    /**
     *@brief Retorna uma mensagem em caso de falha de validação/Pode modificar o
     *conteúdo do campo com um versão do valor corrigida
     *
     * @param fieldname nome do campo
     * @param s valor do campo
     * @return std::optional<std::string>
     */
    virtual auto validate_and_modify(Poco::JSON::Object::Ptr jsondata,
                                     std::string_view fieldname,
                                     const Poco::Dynamic::Var &s)
        -> std::optional<std::string> = 0;

    /**
     *@brief mensagem padrão de falha de validação
     *
     * @param fieldname nome do campo
     * @return std::string mensagem preparada para o usuário
     */
    [[nodiscard]] virtual auto fail_message(std::string_view fieldname) const
        -> std::string = 0;

    InputValidator(const InputValidator &) = default;

    inline InputValidator() {}
    virtual ~InputValidator();
};

/**
 *@brief Validador de email simples baseado em regex
 *@todo Mais testes no padrão de regex usado
 */
class EmailValidator : public InputValidator {

  public:
    auto validate(std::string_view fieldname, const Poco::Dynamic::Var &s)
        -> std::optional<std::string> override {
        if (s.isEmpty()) {
            return std::nullopt;
        }

        const std::regex pattern(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");

        if (!regex_match(s.toString(), pattern)) {
            return fail_message(fieldname);
        }

        return std::nullopt;
    }

    auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                             std::string_view fieldname,
                             const Poco::Dynamic::Var &s)
        -> std::optional<std::string> override {
        return validate(fieldname, s);
    }

    [[nodiscard]] auto fail_message(std::string_view fieldname) const
        -> std::string override {
        return fieldname.data() + std::string(" deve ser um email válido");
    }

    EmailValidator(const EmailValidator &) = default;

    EmailValidator();

    ~EmailValidator() override;
};

/**
 *@brief Validador de inteiro ou conversível válido
 *@todo colocar parâmetros para tratar dados específicos
 */
class IntegerValidator : public InputValidator {

  public:
    auto validate(std::string_view fieldname, const Poco::Dynamic::Var &s)
        -> std::optional<std::string> override {
        if (s.isEmpty()) {
            return std::nullopt;
        }

        try {
            std::stoll(s.toString());
        } catch (const std::exception &) {
            return fail_message(fieldname);
        }

        return std::nullopt;
    }

    auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                             std::string_view fieldname,
                             const Poco::Dynamic::Var &s)
        -> std::optional<std::string> override {
        return validate(fieldname, s);
    }

    [[nodiscard]] auto fail_message(std::string_view fieldname) const
        -> std::string override {
        return fieldname.data() + std::string(" deve ser um inteiro válido");
    }

    IntegerValidator(const IntegerValidator &) = default;

    IntegerValidator() {}

    ~IntegerValidator() override;
};

/**
 *@brief Validador que exige presença de um dado, pode ser qualquer tipo
 *
 */
class RequiredValidator : public InputValidator {

  public:
    auto validate(std::string_view fieldname, const Poco::Dynamic::Var &s)
        -> std::optional<std::string> override {
        if (s.isEmpty()) {
            return fail_message(fieldname);
        }

        return std::nullopt;
    }

    auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                             std::string_view fieldname,
                             const Poco::Dynamic::Var &s)
        -> std::optional<std::string> override {
        return validate(fieldname, s);
    }

    [[nodiscard]] auto fail_message(std::string_view fieldname) const
        -> std::string override {
        return fieldname.data() + std::string(" deve possuir um valor");
    }

    RequiredValidator(const RequiredValidator &) = default;

    RequiredValidator() {}

    ~RequiredValidator() override;
};

template <class T> class InArrayValidator : public InputValidator {
    T dataarray;
    bool empty_pass;

  public:
    auto validate(std::string_view fieldname, const Poco::Dynamic::Var &s)
        -> std::optional<std::string> override {
        if (empty_pass && s.isEmpty()) {
            return fail_message(fieldname);
        }

        try {
            auto it = std::find(dataarray.begin(), dataarray.end(), s);

            if (it == dataarray.end()) {
                return fail_message(fieldname);
            }
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
            return fail_message(fieldname);
        }

        return std::nullopt;
    }

    auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                             std::string_view fieldname,
                             const Poco::Dynamic::Var &s)
        -> std::optional<std::string> override {
        return validate(fieldname, s);
    }

    template <class C,
              typename = std::enable_if_t<std::is_arithmetic<C>::value>>
    static auto convert(C val) -> std::string {
        return std::to_string(val);
    }

    static auto convert(const std::string &val) -> std::string { return val; }
    static auto convert(std::string_view val) -> std::string {
        return val.data();
    }

    [[nodiscard]] auto fail_message(std::string_view fieldname) const
        -> std::string override {
        std::string msg = fieldname.data() +
                          std::string(" deve ser um dos seguintes itens: [");

        bool first = true;

        for (const auto &item : dataarray) {
            if (!first) {
                msg += ", ";
            }

            first = false;

            msg += convert(item);
        }

        msg += "]";

        return msg;
    }

    InArrayValidator(const InArrayValidator &) = default;

    explicit InArrayValidator(T allowed_values, bool allow_empty = false)
        : InputValidator(), dataarray(std::move(allowed_values)),
          empty_pass(allow_empty) {}

    ~InArrayValidator() override {}
};

/**
 *@brief Aplica um valor se o input estiver vazio
 *
 */
class DefaultIfNotPresentValidator : public InputValidator {
    const Poco::Dynamic::Var default_value;

  public:
    /**
     *@brief Does nothing
     *
     * @return std::optional<std::string> std::nullopt
     */
    auto validate(std::string_view /*fieldname*/, const Poco::Dynamic::Var &
                  /*s*/) -> std::optional<std::string> override {
        return std::nullopt;
    }

    /**
     *@brief Modifica o input se não houver o valor padrão
     *
     * @param jsondata json da requisição
     * @param fieldname nome do campo
     * @param s valor do campo
     * @return std::optional<std::string> sempre retorna std::nullopt
     */
    auto validate_and_modify(Poco::JSON::Object::Ptr jsondata,
                             std::string_view fieldname,
                             const Poco::Dynamic::Var &s)
        -> std::optional<std::string> override {
        if (s.isEmpty()) {
            jsondata->set(fieldname.data(), default_value);
        }
        return std::nullopt;
    }

    [[nodiscard]] auto fail_message(std::string_view fieldname) const
        -> std::string override {
        return fieldname.data();
    }

    DefaultIfNotPresentValidator(const DefaultIfNotPresentValidator &) =
        default;

    explicit DefaultIfNotPresentValidator(const Poco::Dynamic::Var &dv)
        : default_value(dv) {}

    ~DefaultIfNotPresentValidator() override;
};

#endif
