#include "DocAPI.hpp"
#include <Poco/UTF8Encoding.h>
#include <Poco/UTF8String.h>

DocAPI::DocAPI() {
    paths = new Poco::JSON::Object;
    mainobj = new Poco::JSON::Object;

    Poco::JSON::Object::Ptr info = new Poco::JSON::Object;
    info->set("contact", Poco::JSON::Object::Ptr(new Poco::JSON::Object));

    Poco::JSON::Object::Ptr components = new Poco::JSON::Object;
    components->set("securitySchemes",
                    Poco::JSON::Object::Ptr(new Poco::JSON::Object));

    info->set("title", "api");
    info->set("version", "1.0.0");
    mainobj->set("info", info);
    mainobj->set("openapi", "3.0.0");
    mainobj->set("servers", Poco::JSON::Array::Ptr(new Poco::JSON::Array));
    mainobj->set("components", components);
}

void DocAPI::set_contact(const std::string &name, const std::string &email,
                         const std::string &url) {
    auto contact = mainobj->getObject("info")->getObject("contact");

    contact->set("name", name);
    contact->set("email", email);
    contact->set("url", url);
}

void DocAPI::add_server(const std::string &description,
                        const std::string &url) {
    auto servers = mainobj->getArray("servers");

    Poco::JSON::Object::Ptr serv = new Poco::JSON::Object;
    serv->set("description", description);
    serv->set("url", url);

    servers->add(serv);
}

void DocAPI::set_api_info_basic(const std::string &title,
                                const std::string &description) {
    auto info = mainobj->getObject("info");

    info->set("title", title);
    info->set("description", description);
}

void DocAPI::register_route_security(const std::string &securityName) {
    auto security = lastroutedata->getArray("security");

    for (const auto &var : *security) {
        auto json = var.extract<Poco::JSON::Object::Ptr>();

        if (json->has(securityName)) {
            return;
        }
    }

    Poco::JSON::Object::Ptr obj = new Poco::JSON::Object;
    obj->set(securityName, Poco::JSON::Array::Ptr(new Poco::JSON::Array));

    security->add(obj);
}

void DocAPI::register_route_description(const std::vector<std::string> &tags,
                                        const std::string &description) {
    lastroutedata->set("description", description);

    Poco::JSON::Array::Ptr jsontags = new Poco::JSON::Array();

    for (const auto &tag : tags) {
        jsontags->add(tag);
    }

    lastroutedata->set("tags", jsontags);
}

void DocAPI::set_security_schema(const std::string &name,
                                 const securitySchemaStruct &description) {
    auto securitySchemes =
        mainobj->getObject("components")->getObject("securitySchemes");

    Poco::JSON::Object::Ptr security = new Poco::JSON::Object;
    security->set("type", description.type);
    security->set("scheme", description.scheme);
    security->set("bearerFormat", description.bearerFormat);

    securitySchemes->set(name, security);
}

auto DocAPI::json_to_swagger(const Poco::Dynamic::Var &obj)
    -> Poco::JSON::Object::Ptr {
    Poco::JSON::Object::Ptr result = new Poco::JSON::Object;
    result->set("description", "");

    if (obj.isArray()) {
        result->set("type", "array");
        try {
            auto object = obj.extract<Poco::JSON::Array::Ptr>();

            Poco::JSON::Object::Ptr items = new Poco::JSON::Object;

            if (object->size() > 0) {
                items->set("example", object->get(0));
            } else {
                items->set("example",
                           Poco::JSON::Object::Ptr(new Poco::JSON::Object));
            }

            items->set("type", "object");

            result->set("items", items);
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
    } else if (obj.isString()) {
        result->set("type", "string");
        result->set("example", obj.toString());
    } else if (obj.isBoolean()) {
        result->set("type", "boolean");
        result->set("example", obj);
    } else if (obj.isInteger()) {
        result->set("type", "integer");
        result->set("example", obj);
    } else if (!obj.isEmpty()) {
        result->set("type", "object");
        try {
            auto object = obj.extract<Poco::JSON::Object::Ptr>();

            Poco::JSON::Object::Ptr properties = new Poco::JSON::Object;

            for (const auto &variable : *object) {
                properties->set(variable.first,
                                json_to_swagger(variable.second));
            }

            result->set("properties", properties);
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
        }
    }

    return result;
}

void DocAPI::register_route_in(const Pistache::Rest::Request &request) {
    std::stringstream ss;
    ss << request.method();
    std::cout << request.query().as_str() << std::endl;
    std::cout << request.resource() << std::endl;

    lastroute = request.resource();
    lastmethod = Poco::UTF8::toLower(ss.str());

    Poco::JSON::Object::Ptr lastroutejs;
    Poco::JSON::Object::Ptr routedata;

    if (paths->has(lastroute)) {
        lastroutejs = paths->getObject(lastroute);
    } else {
        lastroutejs = new Poco::JSON::Object;
        paths->set(lastroute, lastroutejs);
    }

    if (lastroutejs->has(lastmethod)) {
        routedata = lastroutejs->getObject(lastmethod);
    } else {
        routedata = new Poco::JSON::Object;
        lastroutejs->set(lastmethod, routedata);
    }

    lastroutedata = routedata;

    lastroutedata->set("description", "Teste");

    if (!lastroutedata->has("responses")) {
        Poco::JSON::Object::Ptr empty = new Poco::JSON::Object();
        lastroutedata->set("responses", empty);
    }

    if (!lastroutedata->has("security")) {
        Poco::JSON::Array::Ptr empty = new Poco::JSON::Array();
        lastroutedata->set("security", empty);
    }

    const auto queryparameters = request.query().parameters();

    if (!queryparameters.empty()) {
        if (!lastroutedata->has("parameters")) {
            Poco::JSON::Array::Ptr parameters = new Poco::JSON::Array();
            lastroutedata->set("parameters", parameters);

            for (const auto &queryp : queryparameters) {
                Poco::JSON::Object::Ptr param = new Poco::JSON::Object();

                param->set("in", "query");
                param->set("name", queryp);
                param->set("required", true);
                param->set("description", "");

                Poco::JSON::Object::Ptr schema = new Poco::JSON::Object();
                schema->set("type", "string");
                schema->set("example", request.query().get(queryp).value_or(
                                           std::string()));

                param->set("schema", schema);

                parameters->add(param);
            }
        }
    }
}

void DocAPI::register_route_body(const Poco::JSON::Object::Ptr &json) {
    if (lastroutedata.isNull()) {
        return;
    }

    Poco::JSON::Object::Ptr requestBody = new Poco::JSON::Object();
    requestBody->set("required", true);
    requestBody->set("content", content_schema_json(json));

    lastroutedata->set("requestBody", requestBody);
}

auto DocAPI::content_schema_json(const Poco::JSON::Object::Ptr &json)
    -> Poco::JSON::Object::Ptr {
    Poco::JSON::Object::Ptr copy = new Poco::JSON::Object(*json);

    Poco::JSON::Object::Ptr appjson = new Poco::JSON::Object;
    appjson->set("schema", json_to_swagger(copy));

    Poco::JSON::Object::Ptr content = new Poco::JSON::Object;
    content->set("application/json", appjson);

    return content;
}

void DocAPI::register_route_param(const std::string & /*unused*/) {}

void DocAPI::register_route_resp_json(Pistache::Http::Code code,
                                      const Poco::JSON::Object::Ptr &json) {
    if (lastroutedata.isNull()) {
        return;
    }

    auto responses =
        lastroutedata->get("responses").extract<Poco::JSON::Object::Ptr>();

    if (responses.isNull()) {
        return;
    }

    Poco::JSON::Object::Ptr codejson = new Poco::JSON::Object;

    codejson->set("content", content_schema_json(json));
    codejson->set("description", "");

    responses->set(std::to_string(static_cast<int>(code)), codejson);
}

void DocAPI::dump() {

    if (!log.is_open()) {
        log.open("openapi.json", std::ios::trunc | std::ios::out);
    }

    mainobj->set("paths", paths);
    mainobj->stringify(log, 4);
    log << std::endl;

    log.close();
}

auto DocAPI::singleton() -> DocAPI & {
    static DocAPI docs;
    return docs;
}
