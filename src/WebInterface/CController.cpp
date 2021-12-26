/**
 *@file CController.cpp
 * @author Fabio Rossini Sluzala ()
 * @brief
 * @version 0.1
 * @date 2021-05-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "CController.hpp"
#include "../utils/DocAPI.hpp"
#include "../utils/Validator.hpp"
#include <Poco/Crypto/CryptoException.h>
#include <exception>
#include <fstream>

const Pistache::Http::Mime::MediaType CController::JSON_RETURN =
    Pistache::Http::Mime::MediaType(Pistache::Http::Mime::Type::Application,
                                    Pistache::Http::Mime::Subtype::Json,
                                    Pistache::Http::Mime::Suffix::None);

void CController::returnPocoJson(Pistache::Http::Code code,
                                 const Poco::JSON::Object::Ptr &json,
                                 Pistache::Http::ResponseWriter &response) {
    DOCAPI_RESPONSE_JSON(code, json);

    std::stringstream out;
    json->stringify(out);

    response.send(code, out.str(), JSON_RETURN);
}

auto CController::default_json_return(bool success, const std::string &msg)
    -> Poco::JSON::Object::Ptr {
    Poco::JSON::Object::Ptr result(new Poco::JSON::Object);

    result->set("sucesso", success);
    result->set("mensagem", msg);

    return result;
}

auto CController::default_json_return(bool success, const std::string &msg, const Poco::UUID &uuid)
    -> Poco::JSON::Object::Ptr {
    Poco::JSON::Object::Ptr result(new Poco::JSON::Object);

    result->set("sucesso", success);
    result->set("mensagem", msg);
    result->set("req_uuid", uuid);

    return result;
}

auto CController::default_json_return_as_str(bool success,
                                             const std::string &msg)
    -> std::string {

    auto json = default_json_return(success, msg);
    std::stringstream out;
    json->stringify(out);

    return out.str();
}

auto CController::get_ip_host_from_header(
    const Pistache::Rest::Request &request)
    -> std::pair<std::string, std::string> {
    /**
     *@brief verifica e retorna as seguintes headers:
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
     *
     */
    std::pair<std::string, std::string> result;

    try {
        auto xrealip = request.headers().getRaw("X-Real-IP");
        result.first = xrealip.value();
    } catch (const std::exception &e) {
        std::cerr << "X-Real-IP header não existe" << std::endl;
        std::cerr << e.what() << '\n';
    }

    try {

        auto host = request.headers().getRaw("Host");
        result.second = host.value();
    } catch (const std::exception &e) {
        std::cerr << "Host header não existe" << std::endl;
        std::cerr << e.what() << '\n';
    }

    return result;
}

auto CController::get_ip_host_from_request(
    const Pistache::Rest::Request &request)
    -> std::pair<std::string, std::string> {
    std::pair<std::string, std::string> result;

    /**
     * @brief verifica IP cliente pela header em caso de proxy reverso
     via nginx ou do request do Pistache
     *
     */

    std::string pistachehost = request.address().host();
    uint16_t pistacheport = request.address().port();

    /**
     *@brief Lê a header X-Real-IP e Host
     *
     */
    result = get_ip_host_from_header(request);

    /**
     *@brief Verifica se o endereço está vazio ou é localhost
     *
     */
    if (!result.first.empty() &&
        (pistachehost.empty() || pistachehost.find_first_of("127.") == 0)) {
        return result;
    }

    result.first.clear();
    result.first += pistachehost;
    result.first += ":";
    result.first += std::to_string(pistacheport);

    return result;
}

void CController::throw_json_http_exception(
    Pistache::Http::Code code, bool success, const std::string &msg,
    Pistache::Http::ResponseWriter &response) {
    response.setMime(JSON_RETURN);
    returnPocoJson(code, default_json_return(success, msg), response);
    throw std::invalid_argument("");
}

void CController::throw_http_exception(Pistache::Http::Code code,
                                       const std::string &fullreturndata) {
    throw Pistache::Http::HttpError(code, fullreturndata);
}

auto CController::input_json(httpStreamPack httpdata)
    -> Poco::JSON::Object::Ptr {
    try {
        Poco::JSON::Object::Ptr inputdata = Validator::request_to_json(
            httpdata.request, min_json_body_size, max_json_body_size);

        /**
         *@brief Checagem da hash dos dados do json
         *
         */
        if (habilita_hash_input_json &&
            !valida_hash_request(inputdata, httpdata.response)) {
            return nullptr;
        }

        return inputdata;
    } catch (const ValidatorException &e) {
        returnPocoJson(Pistache::Http::Code::Bad_Request, e.to_json(),
                       httpdata.response);
    } catch (const std::exception &) {
        throw_json_http_exception(Pistache::Http::Code::Bad_Request, false,
                                  "Falha de leitura do Json",
                                  httpdata.response);
    }

    return nullptr;
}

auto CController::valida_hash_request(const Poco::JSON::Object::Ptr &param,
                                      Pistache::Http::ResponseWriter &response)
    -> bool {
    if (!param->has(defaulthashfield)) {
        throw_json_http_exception(Pistache::Http::Code::Bad_Request, false,
                                  "Campo de hash \"" + defaulthashfield +
                                      "\" não está presente no json",
                                  response);
    }

    try {
        if (!hash_json_cmp(param, defaulthashfield)) {
            throw_json_http_exception(Pistache::Http::Code::Bad_Request, false,
                                      "Hash dos dados é inválida", response);
        }
    } catch (const Poco::Crypto::CryptoException &) {
        throw_json_http_exception(
            Pistache::Http::Code::Bad_Request, false,
            std::string("Houve um problema com ao testar a hash"), response);
    } catch (const std::exception &e) {
        throw_json_http_exception(Pistache::Http::Code::Bad_Request, false,
                                  e.what(), response);
    }

    return true;
}

auto CController::get_auth(httpStreamPack httpdata)
    -> std::optional<std::string> {
    const auto auth_header =
        httpdata.request.headers()
            .tryGet<Pistache::Http::Header::Authorization>();

    if (!auth_header) {
        returnPocoJson(Pistache::Http::Code::Bad_Request,
                       default_json_return(
                           false, "Falta header Authorization com sua chave"),
                       httpdata.response);
        return std::nullopt;
    }

    const std::optional<std::string> result = auth_header->value();

    if (result.value_or(std::string()).empty()) {
        returnPocoJson(Pistache::Http::Code::Bad_Request,
                       default_json_return(false, "ID de acesso está vazio"),
                       httpdata.response);
        return std::nullopt;
    }

    return result;
}

auto CController::hash_json(const Poco::JSON::Object::Ptr &param,
                            const std::string &ignorefield)
    -> Poco::DigestEngine::Digest {
    Poco::Crypto::DigestEngine SHA("SHA512");
    for (const auto &p : *param.get()) {
        if (p.first == ignorefield) {
            continue;
        }

        SHA.update(p.second.toString());
    }

    return SHA.digest();
}

auto CController::hash_json_cmp(const Poco::JSON::Object::Ptr &param,
                                const std::string &hashfield) -> bool {
    Poco::DigestEngine::Digest target = hash_json(param, hashfield);
    Poco::DigestEngine::Digest source;

    try {
        source = Poco::Crypto::DigestEngine::digestFromHex(
            param->getValue<std::string>(hashfield));
    } catch (...) {
        throw std::runtime_error(
            "Houve um problema em identificar o formato do campo " + hashfield);
    }

    return target == source;
}

auto CController::response_file(const std::string &fullpath,
                                Pistache::Http::ResponseWriter &response)
    -> bool {
    /**
     *@brief Pistache serve file estava com problema quando executado após
     *alguns segundos, esse código resolve enviando o zip em um stream
     * considerei enviar todo o arquivo de uma vez mas isso iria custar carregar
     *ele totalmente na memória de uma vez
     *@todo investigar o motivo do servefile quebrar após alguns segundos de
     *espera do código anterior
     *
     */

    std::ifstream file_stream(fullpath, std::ios::binary);
    file_stream.seekg(0, std::ios::end);
    auto filesize = file_stream.tellg();
    file_stream.seekg(0);

    response.setMime(Pistache::Http::Mime::MediaType(
        Pistache::Http::Mime::Type::Application,
        Pistache::Http::Mime::Subtype::Ext, Pistache::Http::Mime::Suffix::Zip));

    auto stream = response.stream(Pistache::Http::Code::Ok,
                                  static_cast<size_t>(filesize));

    /**
     *@brief Executa a cópia do ifstream para a saída do pistache
     *
     * @todo pesquisar uma maneira mais eficiente com chunks e/ou melhorar a
     *implementação do pistache e enviar como pull request
     */
    for (auto it = std::istreambuf_iterator<char>(file_stream),
              end = std::istreambuf_iterator<char>();
         it != end; it++) {
        char ch = *it;
        stream.write(&ch, 1);
    }
    stream << Pistache::Http::ends;

    return true;
}

auto CController::should_response_html(const Pistache::Rest::Request &request)
    -> bool {
    const auto AcceptHeader =
        request.headers().tryGet<Pistache::Http::Header::Accept>();

    bool shouldShowHtml = !AcceptHeader;

    if (AcceptHeader) {
        auto accept_media = AcceptHeader->media();

        for (const auto &media : accept_media) {
            shouldShowHtml |=
                media.sub() == Pistache::Http::Mime::Subtype::Html;
        }
    }

    return shouldShowHtml;
}

void CController::register_routes(const std::string & /*unused*/,
                                  Pistache::Rest::Router & /*unused*/) {
    throw std::runtime_error(
        "CController::register_routes should not be called");
}
