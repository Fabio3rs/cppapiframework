#pragma once

#include <pistache/net.h>
#include <utility>

#include "pistache.hpp"

namespace httpwrappers {

using Ipv4 = Pistache::Ipv4;
using ReqRaw = const Pistache::Rest::Request &;
using RespRaw = Pistache::Http::ResponseWriter;
using Code = Pistache::Http::Code;
using Address = Pistache::Address;
using MediaType = Pistache::Http::Mime::MediaType;
using Cookie = Pistache::Http::Cookie;
using cstringref = const std::string &;

inline auto jsonMimeType() {
    return MediaType(Pistache::Http::Mime::Type::Application,
                     Pistache::Http::Mime::Subtype::Json,
                     Pistache::Http::Mime::Suffix::None);
}

class Resp;
class Req;

class Req {
    friend class Session;
    friend class WebApp;

  public:
    ReqRaw raw;
    Resp *resp = nullptr;

    explicit Req(ReqRaw requestRaw) : raw(requestRaw) {}
};

class Resp {
    friend class Session;
    friend class WebApp;

  public:
    Code RouteStatus = Code::Ok;
    Pistache::Http::ResponseWriter &response;

    auto status(Code val) -> Resp & {
        RouteStatus = val;
        return *this;
    }

    auto send(const std::string &body, const MediaType &mime = MediaType())
        -> Resp & {
        response.send(RouteStatus, body, mime);
        return *this;
    }

    auto send(Code status, const std::string &body,
              const MediaType &mime = MediaType()) -> Resp & {
        RouteStatus = status;
        response.send(status, body, mime);
        return *this;
    }

    explicit Resp(Pistache::Http::ResponseWriter &res) : response(res) {}
};

class ResponseViaReturn {

  public:
    virtual void sendResponse(Req req, Resp resp) = 0;

    ResponseViaReturn(const ResponseViaReturn &) = default;
    ResponseViaReturn(ResponseViaReturn &&) = default;

    auto operator=(const ResponseViaReturn &) -> ResponseViaReturn & = default;
    auto operator=(ResponseViaReturn &&) -> ResponseViaReturn & = default;

    ResponseViaReturn();
    virtual ~ResponseViaReturn();
};

class ExceptionResponseViaReturn : public ResponseViaReturn {

  public:
    ExceptionResponseViaReturn(const ExceptionResponseViaReturn &) = default;
    ExceptionResponseViaReturn(ExceptionResponseViaReturn &&) = default;

    auto operator=(const ExceptionResponseViaReturn &)
        -> ExceptionResponseViaReturn & = default;
    auto operator=(ExceptionResponseViaReturn &&)
        -> ExceptionResponseViaReturn & = default;

    explicit ExceptionResponseViaReturn(const std::exception &except) {
        text = except.what();
    }

    void sendResponse(Req req, Resp resp) override {
        if (auto Accept = req.raw.headers().get("Accept")) {
            std::cout << Accept << std::endl;

            resp.send(Code::Internal_Server_Error,
                      R"({"message": "exception"})", jsonMimeType());
        } else {
            resp.send(Code::Internal_Server_Error, text);
        }
    }

    ~ExceptionResponseViaReturn() override;

  private:
    std::string text;
};

class RawStringResponse : public ResponseViaReturn {

  public:
    RawStringResponse(const RawStringResponse &) = default;
    RawStringResponse(RawStringResponse &&) = default;

    auto operator=(const RawStringResponse &) -> RawStringResponse & = default;
    auto operator=(RawStringResponse &&) -> RawStringResponse & = default;

    explicit RawStringResponse(std::string_view response, Code rCode = Code::Ok,
                               MediaType mime = {})
        : text(response), retCode(rCode), mimeType(std::move(mime)) {}

    void sendResponse(Req /*ununsed*/, Resp resp) override {
        resp.send(retCode, text, mimeType);
    }

    ~RawStringResponse() override;

  private:
    std::string text;
    Code retCode;
    MediaType mimeType;
};

class RouterWrapper {

  public:
    using callbackDecl_t = std::unique_ptr<ResponseViaReturn>(Req, Resp);

    auto operator()(const Pistache::Rest::Request &request,
                    Pistache::Http::ResponseWriter response)
        -> Pistache::Rest::Route::Result {
        Resp resp(response);
        Req req(request);

        prepareReqResp(req, resp);

        std::unique_ptr<ResponseViaReturn> responseWrapper;

        try {
            responseWrapper = func(req, resp);
            responseWrapper->sendResponse(req, resp);
        } catch (const std::exception &e) {
            ExceptionResponseViaReturn except(e);
            except.sendResponse(req, resp);
        }

        return Pistache::Rest::Route::Result::Ok;
    }

    static void prepareReqResp(Req &req, Resp &resp) {
        req.resp = &resp;
        // req.session.req = &req;
        // req.session.resp = &resp;
    }

    explicit RouterWrapper(std::function<callbackDecl_t> callback)
        : func(std::move(callback)) {}

  private:
    std::function<callbackDecl_t> func;
};

} // namespace httpwrappers
