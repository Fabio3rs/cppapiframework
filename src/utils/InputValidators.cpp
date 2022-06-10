#include "InputValidators.hpp"
#include "ControllerInputValidator.hpp"
#include <utility>

auto ObjectValidator::validate(std::string_view fieldname,
                               const Poco::Dynamic::Var &valchk)
    -> Poco::Dynamic::Var {
    // Recebemos um valor null
    // TODO: Implementar para controlar essa checagem se retorna erro ou não?
    if (valchk.isEmpty()) {
        return {};
    }

    Poco::JSON::Object::Ptr ptr;

    try {
        ptr = valchk.extract<Poco::JSON::Object::Ptr>();
    } catch (const std::exception & /***/) {
        // std::cerr << e.what() << '\n';
        return fail_message(fieldname);
    }

    if (ptr.isNull()) {
        return fail_message(fieldname);
    }

    ControllerInputValidator inputval(ptr);

    validatefn(fieldname, inputval);

    auto responsedata = inputval.get_only_messages();

    if (responsedata.isNull()) {
        return {};
    }

    return responsedata;
}

auto ArrayValidator::validate(std::string_view fieldname,
                              const Poco::Dynamic::Var &valchk)
    -> Poco::Dynamic::Var {
    // Recebemos um valor null
    // TODO: Implementar para controlar essa checagem se retorna erro ou não?
    if (valchk.isEmpty()) {
        return {};
    }

    Poco::JSON::Array::Ptr ptr;

    try {
        ptr = valchk.extract<Poco::JSON::Array::Ptr>();
    } catch (const std::exception & /***/) {
        // std::cerr << e.what() << '\n';
        return fail_message(fieldname);
    }

    if (ptr.isNull()) {
        return fail_message(fieldname);
    }

    Poco::JSON::Object::Ptr temporaryObject;
    Poco::Dynamic::Var resultadoValidacao;

    size_t index = 0;
    for (const auto &data : (*ptr)) {
        resultadoValidacao = validatefn(fieldname, index, data);

        // faz algo com o resultado aqui, por exemplo validar por linhas
        if (resultadoValidacao.isEmpty()) {
            if (temporaryObject.isNull()) {
                temporaryObject = new Poco::JSON::Object;
            }

            temporaryObject->set(std::to_string(index), resultadoValidacao);
        }
        ++index;
    }

    return resultadoValidacao;
}

RequiredValidator::~RequiredValidator() = default;

IntegerValidator::~IntegerValidator() = default;

InputValidator::~InputValidator() = default;

EmailValidator::EmailValidator() = default;

EmailValidator::~EmailValidator() = default;

DefaultIfNotPresentValidator::~DefaultIfNotPresentValidator() = default;

ObjectValidator::~ObjectValidator() = default;

ArrayValidator::~ArrayValidator() = default;

StringLengthValidator::~StringLengthValidator() = default;

ObjectValidator::ObjectValidator(std::function<callback_t> cbfn)
    : validatefn(std::move(cbfn)) {}

auto EmailValidator::validate(std::string_view fieldname,
                              const Poco::Dynamic::Var &valchk)
    -> Poco::Dynamic::Var {
    if (valchk.isEmpty()) {
        return {};
    }

    const std::regex pattern(
        R"((?:[A-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[A-z0-9!#$%&'*+/=?^_`{|}~-]+)*|"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*")@(?:(?:[A-z0-9](?:[A-z0-9-]*[A-z0-9])?\.)+[A-z0-9](?:[A-z0-9-]*[A-z0-9])?|\[(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?|[A-z0-9-]*[A-z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\]))");

    if (!regex_match(valchk.toString(), pattern)) {
        return fail_message(fieldname);
    }

    return {};
}
