#pragma once

#include "httpwrappers.hpp"

#include <Poco/JSON/Object.h>
#include <sstream>

namespace httpwrappers {
class JsonResponse : public ResponseViaReturn {

  public:
    JsonResponse(const JsonResponse &) = default;
    JsonResponse(JsonResponse &&) = default;

    auto operator=(const JsonResponse &) -> JsonResponse & = default;
    auto operator=(JsonResponse &&) -> JsonResponse & = default;

    explicit JsonResponse(const Poco::JSON::Object::Ptr &response,
                          Code rCode = Code::Ok)
        : respData(response), retCode(rCode) {}

    void sendResponse(Req /*ununsed*/, Resp resp) override {
        if (respData.isNull()) {
            resp.send(retCode, "{}", jsonMimeType());
        } else {
            std::stringstream sstr;
            try {
                respData->stringify(sstr);
            } catch (...) {
                sstr.str("{}");
            }
            resp.send(retCode, sstr.str(), jsonMimeType());
        }
    }

    ~JsonResponse() override;

  private:
    Poco::JSON::Object::Ptr respData;
    Code retCode;
};
} // namespace httpwrappers
