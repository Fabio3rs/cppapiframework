/**
 *@file ControllerInputModifier.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief Validador e modificador de input
 * @version 0.1
 * @date 2021-07-26
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#ifndef CONTROLLERINPUTMODIFIER_HPP
#define CONTROLLERINPUTMODIFIER_HPP
#include "InputValidators.hpp"

/**
 *@brief Valida e pode modificar o json de input de dados
 *
 */
class ControllerInputModifier {
    Poco::JSON::Object::Ptr parameters;
    Poco::JSON::Object::Ptr resultadofinal;
    Poco::JSON::Object::Ptr resobj;
    bool validation_failed;

  public:
    static void push_validation_msg(const std::optional<std::string> &str,
                                    Poco::JSON::Array &data) {
        if (!str.has_value()) {
            return;
        }

        data.add(str.value());
    }

    void push_val_list(std::string_view fieldname, Poco::JSON::Array arr) {
        if (arr.size() == 0)
            return;

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
     *@brief Chama os validadores indicados do input
     *
     * @tparam Types tipos das classes de validação, devem seguir o padrão da
     *classe InputValidator
     * @param fieldname nome do campo
     * @param args lista de objetos de classe de validação
     * @return true validação falhou
     * @return false o campo passou por todos os testes com sucesso
     */
    template <class... Types>
    auto validate_input(std::string_view fieldname, Types... args) -> bool {
        const Poco::Dynamic::Var data = parameters->get(fieldname.data());
        Poco::JSON::Array result;

        ((push_validation_msg(
             args.validate_and_modify(parameters, fieldname, data), result)),
         ...);

        bool failed = result.size() > 0;
        push_val_list(fieldname, std::move(result));

        return failed;
    }

    /**
     *@brief Lê a resposta dos validadores
     *
     * @return auto Json com dados de validação a ser apresentado ao usuário
     */
    auto get_response() {
        if (!resultadofinal.isNull()) {
            resultadofinal->set("resultado", resobj);
        }
        return resultadofinal;
    }

    ControllerInputModifier(Poco::JSON::Object::Ptr p) : parameters(p) {
        validation_failed = false;
    }
};

#endif
