#include "Validator.hpp"
#include "CConfig.hpp"

auto ValidatorException::to_json() const -> Poco::JSON::Object::Ptr {
    Poco::JSON::Object::Ptr result = new Poco::JSON::Object;

    result->set("sucesso", false);
    result->set("mensagem", what());

    return result;
}

auto ValidatorException::what() const noexcept -> const char * {
    return msg.c_str();
}
