/**
 *@file DocAPI.hpp
 * @author Fabio Rossini Sluzala ()
 * @brief Helper para autodocumentação das rotas da API com swagger. Output json
 *openapi
 * @version 0.1
 * @date 2021-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#ifndef DOCAPI_HPP
#define DOCAPI_HPP

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
#include <fstream>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

class DocAPI {
    std::fstream log;
    std::string lastroute, lastmethod;

    Poco::JSON::Object::Ptr lastroutedata;

    Poco::JSON::Object::Ptr mainobj;
    Poco::JSON::Object::Ptr paths;

    DocAPI();

  public:
    struct securitySchemaStruct {
        std::string type;
        std::string scheme;
        std::string bearerFormat;
    };

    static auto json_to_swagger(const Poco::Dynamic::Var &obj)
        -> Poco::JSON::Object::Ptr;

    static auto json_return_example_to_swagger(Poco::Dynamic::Var obj)
        -> Poco::JSON::Object::Ptr;

    static auto content_schema_json(const Poco::JSON::Object::Ptr &json)
        -> Poco::JSON::Object::Ptr;

    void register_route_in(const Pistache::Rest::Request &request);
    void register_route_body(const Poco::JSON::Object::Ptr &json);
    void register_route_param(const std::string &field);
    void register_route_resp_json(Pistache::Http::Code code,
                                  const Poco::JSON::Object::Ptr &json);
    void register_route_security(const std::string &securityName);
    void register_route_description(const std::vector<std::string> &tags,
                                    const std::string &description);

    void set_contact(const std::string &name, const std::string &email,
                     const std::string &url);
    void add_server(const std::string &description, const std::string &url);
    void set_api_info_basic(const std::string &title,
                            const std::string &description);

    void set_security_schema(const std::string &name,
                             const securitySchemaStruct &description);

    void dump();

    static auto singleton() -> DocAPI &;
};

/**
 *@brief Macros para autodocumentação OpenAPI. Somente acessíveis em debug.
 *
 */
#ifdef NDEBUG
#define DOCAPI_REGISTER_ROUTE_IN(param)
#define DOCAPI_REGISTER_ROUTE_BODY(param)
#define DOCAPI_RESPONSE_JSON(code, json)
#define DOCAPI_REGISTER_ROUTE_SECURITY(param)
#define DOCAPI_REGISTER_ROUTE_DESCRIPTION(tags, desc)
#define DOCAPI_REGISTER_QUERY_DESCRIPTION(query, desc, required)
#else
#define DOCAPI_REGISTER_ROUTE_IN(param)                                        \
    DocAPI::singleton().register_route_in(param)

#define DOCAPI_REGISTER_ROUTE_BODY(param)                                      \
    DocAPI::singleton().register_route_body(param)

#define DOCAPI_RESPONSE_JSON(code, json)                                       \
    DocAPI::singleton().register_route_resp_json(code, json)

#define DOCAPI_REGISTER_ROUTE_SECURITY(param)                                  \
    DocAPI::singleton().register_route_security(param)

#define DOCAPI_REGISTER_ROUTE_DESCRIPTION(tags, desc)                          \
    DocAPI::singleton().register_route_description(tags, desc)

#define DOCAPI_ENABLED 1
#endif

#endif
