#include "projstdafx.hpp"

#include <gtest/gtest.h>

using namespace webapp;

// NOLINTBEGIN(hicpp-special-member-functions)
namespace {
auto request_getstr(const std::string &fullurl)
    -> std::pair<std::string, Poco::Net::HTTPResponse::HTTPStatus> {
    Poco::URI uri(fullurl);

    std::string path(uri.getPathAndQuery());

    if (path.empty()) {
        path = "/";
    }

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, path,
                                   Poco::Net::HTTPMessage::HTTP_1_1);

    request.setContentType("application/json");
    request.set("Accept-Encoding", "gzip");

    auto borrowedSession = CHttpPool::default_inst().setupSession(uri);

    if (!borrowedSession || !(*borrowedSession)) {
        throw std::runtime_error("Falha ao recuperar a sessão HTTP");
    }

    (*borrowedSession)->setTimeout({1, 0});
    (*borrowedSession)->sendRequest(request);

    Poco::Net::HTTPResponse response;
    std::istream &rstr = (*borrowedSession)->receiveResponse(response);

    std::string result;
    Poco::StreamCopier::copyToString(rstr, result);

    return std::make_pair(result, response.getStatus());
}

auto somefn(Req /*ununsed*/, Resp /*ununsed*/)
    -> std::unique_ptr<ResponseViaReturn> {
    return std::make_unique<RawStringResponse>("some raw string 1");
}

void somefnRetVoid(Req req, Resp resp) {
    RawStringResponse toresp("some raw string 2");
    toresp.sendResponse(req, resp);
}

auto mountUrl(const WebApp &app, const std::string &route) -> std::string {
    return Strutils::multi_concat(
        "http://127.0.0.1:", std::to_string(app.getPort()), route);
}

void testGetReqStr(
    const WebApp &app, const std::string &route,
    const std::pair<std::string, Poco::Net::HTTPResponse::HTTPStatus>
        &expected) {
    EXPECT_EQ(request_getstr(mountUrl(app, route)), expected);
}
} // namespace

// NOLINTNEXTLINE
TEST(WebAppTest, Startup) {
    WebApp app({Ipv4::any(), 0}, 2);

    app.get("/", somefn)
        .get("/voidret", somefnRetVoid)
        .get("/voidretlambda", [](Req req, Resp resp) {
            RawStringResponse toresp("some raw string 3");
            toresp.sendResponse(req, resp);
        });

    app.startAsync();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    using HTTPStatus = Poco::Net::HTTPResponse::HTTPStatus;

    testGetReqStr(app, "/", {"some raw string 1", HTTPStatus::HTTP_OK});

    testGetReqStr(app, "/voidret", {"some raw string 2", HTTPStatus::HTTP_OK});

    testGetReqStr(app, "/voidretlambda",
                  {"some raw string 3", HTTPStatus::HTTP_OK});
}

// NOLINTNEXTLINE
TEST(WebAppTest, JsonResult) {
    WebApp app({Ipv4::any(), 0}, 2);

    app.get("/",
            [](Req /*ununsed*/,
               Resp /*ununsed*/) -> std::unique_ptr<ResponseViaReturn> {
                Poco::JSON::Object::Ptr aaaa(new Poco::JSON::Object);
                aaaa->set("teste", true);
                return std::make_unique<JsonResponse>(aaaa);
            });

    app.startAsync();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    using HTTPStatus = Poco::Net::HTTPResponse::HTTPStatus;
    testGetReqStr(app, "/", {R"json({"teste":true})json", HTTPStatus::HTTP_OK});
}

// NOLINTEND(hicpp-special-member-functions)
