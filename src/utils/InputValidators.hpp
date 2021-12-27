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
#include "../stdafx.hpp"

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
  virtual auto validate(std::string_view fieldname, const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var = 0;

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
      -> Poco::Dynamic::Var = 0;

  /**
   *@brief mensagem padrão de falha de validação
   *
   * @param fieldname nome do campo
   * @return std::string mensagem preparada para o usuário
   */
  [[nodiscard]] virtual auto fail_message(std::string_view fieldname) const
      -> std::string = 0;

  // callback for array
  virtual auto operator()(std::string_view fieldname, size_t /*ununsed*/,
                          Poco::Dynamic::Var s) -> Poco::Dynamic::Var {
    return validate(fieldname, s);
  }

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
      -> Poco::Dynamic::Var override {
    if (s.isEmpty()) {
      return Poco::Dynamic::Var();
    }

    const std::regex pattern(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");

    if (!regex_match(s.toString(), pattern)) {
      return fail_message(fieldname);
    }

    return Poco::Dynamic::Var();
  }

  auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                           std::string_view fieldname,
                           const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {
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
      -> Poco::Dynamic::Var override {
    if (s.isEmpty()) {
      return Poco::Dynamic::Var();
    }

    try {
      std::stoll(s.toString());
    } catch (const std::exception &) {
      return fail_message(fieldname);
    }

    return Poco::Dynamic::Var();
  }

  auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                           std::string_view fieldname,
                           const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {
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
      -> Poco::Dynamic::Var override {
    if (s.isEmpty()) {
      return fail_message(fieldname);
    }

    return Poco::Dynamic::Var();
  }

  auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                           std::string_view fieldname,
                           const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {
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

class ControllerInputValidator;

class ObjectValidator : public InputValidator {
  typedef void(callback_t)(std::string_view, ControllerInputValidator &);

  std::function<callback_t> validatefn;

public:
  auto validate(std::string_view fieldname, const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override;

  auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                           std::string_view fieldname,
                           const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {
    return validate(fieldname, s);
  }

  [[nodiscard]] auto fail_message(std::string_view fieldname) const
      -> std::string override {
    return fieldname.data() + std::string(" deve ser um objeto");
  }

  ObjectValidator(const ObjectValidator &) = default;

  ObjectValidator(std::function<callback_t> cb)
      : InputValidator(), validatefn(cb) {}

  ~ObjectValidator() override;
};

/**
 * @brief Valida campos do tipo Array
 *
 */
class ArrayValidator : public InputValidator {

  typedef Poco::Dynamic::Var(callback_t)(std::string_view, size_t,
                                         Poco::Dynamic::Var);
  std::function<callback_t> validatefn;

public:
  auto validate(std::string_view fieldname, const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override;

  auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                           std::string_view fieldname,
                           const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {
    return validate(fieldname, s);
  }

  [[nodiscard]] auto fail_message(std::string_view fieldname) const
      -> std::string override {
    return fieldname.data() + std::string(" deve ser um array");
  }

  ArrayValidator(const ArrayValidator &) = default;

  ArrayValidator(std::function<callback_t> cb)
      : InputValidator(), validatefn(cb) {}

  ~ArrayValidator() override;
};

template <class... Types> class OrValidator : public InputValidator {
  std::tuple<Types...> rules;

  static void validate_or(InputValidator &val, std::string_view fieldname,
                          const Poco::Dynamic::Var &s, bool &success,
                          Poco::JSON::Array::Ptr &resultInfo) {
    if (success) {
      return; // sucesso já, não precisa dar push
    }

    auto valres = val.validate(fieldname, s);
    if (valres.isEmpty()) {
      success = true;
    } else {
      if (resultInfo.isNull()) {
        resultInfo = new Poco::JSON::Array;
      }

      resultInfo->add(valres);
    }
  }

public:
  auto validate(std::string_view fieldname, const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {
    bool success = false;
    Poco::JSON::Array::Ptr resultInfo;

    std::apply(
        [fieldname, &s, &resultInfo, &success](auto &&... args) {
          ((validate_or(args, fieldname, s, success, resultInfo)), ...);
        },
        rules);

    if (!success) {
      Poco::JSON::Object::Ptr result(new Poco::JSON::Object);

      result->set("mensagem",
                  "Valor inválido, deve atender uma das regras a seguir:");
      result->set("regras", resultInfo);

      return result;
    }

    return Poco::Dynamic::Var();
  }

  auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                           std::string_view fieldname,
                           const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {
    return validate(fieldname, s);
  }

  [[nodiscard]] auto fail_message(std::string_view fieldname) const
      -> std::string override {
    return fieldname.data() + std::string(" deve possuir um valor");
  }

  OrValidator(const OrValidator &) = default;

  OrValidator(Types &&... args)
      : InputValidator(), rules(std::forward<Types>(args)...) {}

  ~OrValidator() override {}
};

class StringLengthValidator : public InputValidator {
  size_t min;
  size_t max;

public:
  auto validate(std::string_view fieldname, const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {

    if (s.isEmpty()) {
      return fail_message(fieldname);
    }

    std::string str = s.toString();
    if (str.size() == 0) {
      return fail_message_empty(fieldname);
    }
    if (str.size() >= min && str.size() <= max) {
      return Poco::Dynamic::Var();
    }
    return fail_message(fieldname);
  }

  auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                           std::string_view fieldname,
                           const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {
    return validate(fieldname, s);
  }

  [[nodiscard]] auto fail_message(std::string_view fieldname) const
      -> std::string override {
    std::string fail_message;
    fail_message.reserve(32);

    fail_message.append(fieldname);
    fail_message.append(" deve ter entre ");
    fail_message.append(std::to_string(min));
    fail_message.append(" e ");
    fail_message.append(std::to_string(max));
    fail_message.append(" caracteres");

    return fail_message;
  }

  auto fail_message_empty(std::string_view fieldname) const -> std::string {
    std::string fail_message;
    fail_message.reserve(16);

    fail_message.append(fieldname);
    fail_message.append(" não pode ser vazio");

    return fail_message;
  }

  StringLengthValidator(const StringLengthValidator &) = default;

  explicit StringLengthValidator(size_t minimum, size_t maximum)
      : InputValidator(), min(minimum), max(maximum) {}

  explicit StringLengthValidator()
      : InputValidator(), min(1), max(std::numeric_limits<size_t>::max()) {}

  ~StringLengthValidator() override;
};

template <class T> class InArrayValidator : public InputValidator {
  T dataarray;
  bool empty_pass;

public:
  auto validate(std::string_view fieldname, const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {
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

    return Poco::Dynamic::Var();
  }

  auto validate_and_modify(Poco::JSON::Object::Ptr /*jsondata*/,
                           std::string_view fieldname,
                           const Poco::Dynamic::Var &s)
      -> Poco::Dynamic::Var override {
    return validate(fieldname, s);
  }

  template <class C, typename = std::enable_if_t<std::is_arithmetic<C>::value>>
  static auto convert(C val) -> std::string {
    return std::to_string(val);
  }

  static auto convert(const std::string &val) -> std::string { return val; }
  static auto convert(std::string_view val) -> std::string {
    return val.data();
  }

  [[nodiscard]] auto fail_message(std::string_view fieldname) const
      -> std::string override {
    std::string msg =
        fieldname.data() + std::string(" deve ser um dos seguintes itens: [");

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
                /*s*/) -> Poco::Dynamic::Var override {
    return Poco::Dynamic::Var();
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
      -> Poco::Dynamic::Var override {
    if (s.isEmpty()) {
      jsondata->set(fieldname.data(), default_value);
    }
    return Poco::Dynamic::Var();
  }

  [[nodiscard]] auto fail_message(std::string_view fieldname) const
      -> std::string override {
    return fieldname.data();
  }

  DefaultIfNotPresentValidator(const DefaultIfNotPresentValidator &) = default;

  explicit DefaultIfNotPresentValidator(const Poco::Dynamic::Var &dv)
      : default_value(dv) {}

  ~DefaultIfNotPresentValidator() override;
};

#endif
