#include "WebInputValidator.hpp"
#include "CController.hpp"

auto WebInputValidator::validate() -> bool {
    /**
     *@brief Chama o callback de validação
     *
     */
    if (fn_obj) {
        fn_obj(inputValidator);
    }

    /**
     *@brief Se houver erros de validação envia o retorno ao cliente, deve
     *retornar a rota a partir disso
     *
     */
    auto responsedata = inputValidator.get_response();
    if (!responsedata.isNull()) {
        CController::returnPocoJson(Pistache::Http::Code::Bad_Request,
                                    responsedata, response);
        return false;
    }

    return true;
}
