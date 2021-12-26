/**
 *@file WebInputValidator.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief Desenhado para validar campos json com pistache
 * @version 0.1
 * @date 2021-08-04
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#ifndef WEBINPUTVALIDATOR_HPP
#define WEBINPUTVALIDATOR_HPP

#include "../utils/ControllerInputValidator.hpp"
#include <functional>
#include <utility>

#include "pistache.hpp"

class WebInputValidator {
    ControllerInputValidator inputValidator;
    Pistache::Http::ResponseWriter &response;
    std::function<void(ControllerInputValidator &)> fn_obj;

  public:
    /**
     *@brief Efetura a validação dos campos do json e seta o retorno em caso de
     *falha
     *
     * @return true validado com sucesso
     * @return false falha de validação. Response setado com os erros de
     *validação. Rota deve retornar
     */
    auto validate() -> bool;

    /**
     *@brief Construct a new Web Input Validator object
     *
     * @param parameters dados que chegaram no body da rota parseado para objeto
     *json
     * @param resp objeto de resposta do pistache
     * @param cb callback para efetuar as validações do usando um objeto
     *ControllerInputValidator
     */
    WebInputValidator(const Poco::JSON::Object::Ptr &parameters,
                      Pistache::Http::ResponseWriter &resp,
                      std::function<void(ControllerInputValidator &)> cb)
        : inputValidator(parameters), response(resp), fn_obj(std::move(cb)) {}
};

#endif
