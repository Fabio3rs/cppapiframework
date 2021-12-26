/**
 *@file ControllerInputValidator.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief Implementação de templates para rodar a validação de campos de json
 * @version 0.1
 * @date 2021-08-04
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#ifndef CONTROLLERINPUTVALIDATOR_HPP
#define CONTROLLERINPUTVALIDATOR_HPP
#include "InputValidators.hpp"

class ControllerInputValidator {
    const Poco::JSON::Object::Ptr parameters;
    Poco::JSON::Object::Ptr resultadofinal;
    Poco::JSON::Object::Ptr resobj;
    bool validation_failed;

  public:
    /**
     *@brief pequeno helper que recebe um pair e guarda o first como
     *std::string_view, criado para ser usado em conjunto com
     *find_values_not_in_list para testar se um campo json está fora da lista de
     *validação
     *
     */
    struct pair_first {
        std::string_view str;

        auto operator==(const std::string &strtocmp) const -> bool {
            return strtocmp == str;
        }

        template <class P> pair_first(const P &pair) : str(pair.first) {}
    };

    /**
     *@brief Guarda a mensagem de validação no array
     *
     * @param str string da mensagem de validação
     * @param data objeto array de dados
     */
    static void push_validation_msg(const std::optional<std::string> &str,
                                    Poco::JSON::Array &data) {
        if (!str.has_value()) {
            return;
        }

        data.add(str.value());
    }

    /**
     *@brief Registra mensagens de erro de validação
     *
     * @param fieldname nome do campo
     * @param arr lista de mensagens de erro
     */
    void push_val_list(std::string_view fieldname,
                       const Poco::JSON::Array &arr) {
        if (arr.size() == 0) {
            return;
        }

        /**
         *@brief "Lazy pointer", alocando apenas se houver necessidade
         *
         */
        if (resultadofinal.isNull()) {
            resultadofinal = new Poco::JSON::Object;
            resultadofinal->set("sucesso", false);
            resultadofinal->set("mensagem",
                                "Falha na validação de um ou mais campos");
        }

        if (resobj.isNull()) {
            resobj = new Poco::JSON::Object;
        }

        validation_failed = true;
        resobj->set(fieldname.data(), arr);
    }

    /**
     *@brief Efetua validação de um campo específico do json e grava os erros de
     *validação
     *
     * @tparam Types InputValidator
     * @param fieldname nome do campo
     * @param args sequencia de objetos de validadores para serem testados com
     *os dados do campo
     * @return true um ou mais validadores falharam
     * @return false não houveram falhas
     */
    template <class... Types>
    auto validate_input(std::string_view fieldname, Types... args) -> bool {
        const Poco::Dynamic::Var data = parameters->get(fieldname.data());
        Poco::JSON::Array result;

        ((push_validation_msg(args.validate(fieldname, data), result)), ...);

        bool failed = result.size() > 0;
        push_val_list(fieldname, std::move(result));

        return failed;
    }

    template <class N, class T>
    auto validate_input_custom(N name, T data) -> bool {
        return std::apply(
            [&](auto &&... validation_list) {
                return validate_input(name, validation_list...);
            },
            data);
    }

    /**
     *@brief Valida se o json possui campos que não estão na lista de validação
     *
     * @tparam Types std::pair(std::string_view, {...})...
     * @param args std::pair(nome_campo, std::tuple{ validadores })...
     * @return auto .
     */
    template <class... Types> auto find_values_not_in_list(Types... args) {
        const std::array<pair_first,
                         std::tuple_size<std::tuple<Types...>>::value>
            a = {std::forward<Types>(args)...};

        for (const auto &item : *parameters) {
            auto it = std::find(a.begin(), a.end(), item.first);

            if (it == a.end()) {
                Poco::JSON::Array resultmsg;
                resultmsg.add("Não é um parâmetro reconhecido");

                push_val_list(item.first, resultmsg);
            }
        }
    }

    /**
     *@brief Valida os tipos dos parâmetros do json e valida se há parâmetros
     *desconhecidos
     *
     * @tparam Types std::pair(std::string_view, {...})...
     * @param args std::pair(nome_campo, std::tuple{ validadores })...
     * @return true ocorreu um erro
     * @return false não foi encontrado problema
     */
    template <class... Types> auto full_validation(Types... args) -> bool {
        bool result = false;

        ((result |= validate_input_custom(args.first, args.second)), ...);

        find_values_not_in_list(std::forward<Types>(args)...);

        return result;
    }

    /**
     *@brief Recebe um json de resposta ao usuário
     *
     * @return auto Poco::JSON::Object::Ptr
     */
    auto get_response() {
        if (!resultadofinal.isNull()) {
            resultadofinal->set("resultado", resobj);
        }
        return resultadofinal;
    }

    /**
     *@brief Construct a new Controller Input Validator object
     *
     * @param p json com os dados para serem validados
     */
    explicit ControllerInputValidator(const Poco::JSON::Object::Ptr &p)
        : parameters(p) {
        validation_failed = false;
    }
};

#endif
