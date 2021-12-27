/**
 *@file CController.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief Controller base class for use with CPistacheEndpoint
 * @version 0.1
 *
 */
#pragma once
#ifndef CController_hpp
#define CController_hpp

#include "../stdafx.hpp"
#include "pistache.hpp"

struct httpStreamPack {
    const Pistache::Rest::Request &request;
    Pistache::Http::ResponseWriter &response;

    httpStreamPack(const Pistache::Rest::Request &req,
                   Pistache::Http::ResponseWriter &resp)
        : request(req), response(resp) {}
};

/**
 *@brief CControler base class
 *
 */
class CController {
    size_t min_json_body_size{0}, max_json_body_size{4 * 1024};

  protected:
    /**
     *@brief Nome do campo de hash do input json
     *
     */
    std::string defaulthashfield = "_hash";

    /**
     *@brief Ativa ou não a checagem de hash
     *
     */
    bool habilita_hash_input_json = false;

  public:
    using msg_pair_t = std::pair<bool, std::string>;

    const static Pistache::Http::Mime::MediaType JSON_RETURN;

    static void returnPocoJson(Pistache::Http::Code code,
                               const Poco::JSON::Object::Ptr &json,
                               Pistache::Http::ResponseWriter &response);

    static auto get_ip_host_from_header(const Pistache::Rest::Request &request)
        -> std::pair<std::string, std::string>;
    static auto get_ip_host_from_request(const Pistache::Rest::Request &request)
        -> std::pair<std::string, std::string>;

    auto input_json(httpStreamPack httpdata) -> Poco::JSON::Object::Ptr;
    static auto get_auth(httpStreamPack httpdata) -> std::optional<std::string>;
    static auto default_json_return(bool success, const std::string &msg)
        -> Poco::JSON::Object::Ptr;

    static auto default_json_return(bool success, const std::string &msg,
                                    const Poco::UUID &uuid)
        -> Poco::JSON::Object::Ptr;

    static auto default_json_return_as_str(bool success, const std::string &msg)
        -> std::string;

    static void throw_json_http_exception
        [[noreturn]] (Pistache::Http::Code code, bool success,
                      const std::string &msg,
                      Pistache::Http::ResponseWriter &response);

    static void throw_http_exception
        [[noreturn]] (Pistache::Http::Code code,
                      const std::string &fullreturndata);

    auto valida_hash_request(const Poco::JSON::Object::Ptr &param,
                             Pistache::Http::ResponseWriter &response) -> bool;

    static auto hash_json(const Poco::JSON::Object::Ptr &param,
                          const std::string &ignorefield)
        -> Poco::DigestEngine::Digest;

    static auto hash_json_cmp(const Poco::JSON::Object::Ptr &param,
                              const std::string &hashfield) -> bool;

    static auto response_file(const std::string &fullpath,
                              Pistache::Http::ResponseWriter &response) -> bool;

    static auto should_response_html(const Pistache::Rest::Request &request)
        -> bool;

    template <class stream_t>
    auto response_stream(const stream_t &inputstream,
                         Pistache::Http::ResponseWriter &response) -> bool {
        inputstream.seekg(0, std::ios::end);
        auto filesize = inputstream.tellg();
        inputstream.seekg(0, std::ios::beg);

        response.setMime(Pistache::Http::Mime::MediaType(
            Pistache::Http::Mime::Type::Application,
            Pistache::Http::Mime::Subtype::Ext,
            Pistache::Http::Mime::Suffix::Zip));

        auto stream = response.stream(Pistache::Http::Code::Ok,
                                      static_cast<size_t>(filesize));

        /**
         *@brief Executa a cópia do ifstream para a saída do pistache
         *
         * @todo pesquisar uma maneira mais eficiente com chunks e/ou melhorar a
         *implementação do pistache e enviar como pull request
         */
        for (auto it = std::istreambuf_iterator<char>(inputstream),
                  end = std::istreambuf_iterator<char>();
             it != end; it++) {
            char ch = *it;
            stream.write(&ch, 1);
        }
        stream << Pistache::Http::ends;

        return true;
    }

    /**
     *@brief Function CPistacheEndpoint calls to the controller register the
     *routes
     *
     */
    virtual void register_routes(const std::string & /*unused*/,
                                 Pistache::Rest::Router & /*unused*/);

    auto operator=(const CController &) -> CController & = delete;
    auto operator=(CController &&) -> CController & = delete;

    /**
     *@brief Ativa ou desativa a checagem de hash no input json das rotas
     *
     * @param e true - ativa a checagem de hash
     */
    void enableInputHashCheck(bool e) { habilita_hash_input_json = e; }

    template <class T>
    void route_get(Pistache::Rest::Router &router, const std::string &routepath,
                   T routefun) {
        Pistache::Rest::Routes::Get(router, routepath, routefun);
    }

    template <class T>
    void route_post(Pistache::Rest::Router &router,
                    const std::string &routepath, T routefun) {
        Pistache::Rest::Routes::Post(router, routepath, routefun);
    }

    CController() = default;
    CController(const CController &) = default;
    CController(CController &&) = default;

    virtual ~CController() = default;
};

#endif
