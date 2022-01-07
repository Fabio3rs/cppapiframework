#include "InputValidators.hpp"
#include "ControllerInputValidator.hpp"

auto ObjectValidator::validate(std::string_view fieldname,
                               const Poco::Dynamic::Var &s)
    -> Poco::Dynamic::Var {
    // Recebemos um valor null
    // TODO: Implementar para controlar essa checagem se retorna erro ou não?
    if (s.isEmpty()) {
        return Poco::Dynamic::Var();
    }

    Poco::JSON::Object::Ptr ptr;

    try {
        ptr = s.extract<Poco::JSON::Object::Ptr>();
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
        return Poco::Dynamic::Var();
    }

    return responsedata;
}

auto ArrayValidator::validate(std::string_view fieldname,
                              const Poco::Dynamic::Var &s)
    -> Poco::Dynamic::Var {
    // Recebemos um valor null
    // TODO: Implementar para controlar essa checagem se retorna erro ou não?
    if (s.isEmpty()) {
        return Poco::Dynamic::Var();
    }

    Poco::JSON::Array::Ptr ptr;

    try {
        ptr = s.extract<Poco::JSON::Array::Ptr>();
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

RequiredValidator::~RequiredValidator() {}

IntegerValidator::~IntegerValidator() {}

InputValidator::~InputValidator() {}

EmailValidator::EmailValidator() {}

EmailValidator::~EmailValidator() {}

DefaultIfNotPresentValidator::~DefaultIfNotPresentValidator() {}

ObjectValidator::~ObjectValidator() {}

ArrayValidator::~ArrayValidator() {}

StringLengthValidator::~StringLengthValidator() {}
