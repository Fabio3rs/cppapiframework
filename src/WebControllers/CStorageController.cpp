#include "CStorageController.hpp"
#include "../PistacheCustomHttpHeaders/LastModified.hpp"
#include <filesystem>
#include <fstream>
#include <pistache/http_defs.h>
#include <string>
#include <string_view>
#include <system_error>

using namespace Pistache::Http;

auto CStorageController::getFilePath(
    const Pistache::Rest::Request &request) const -> std::filesystem::path {
    std::filesystem::path fileName;
    try {
        auto fileNameStr = request.param(":fileName").as<std::string>();

        if (fileNameStr.at(0) == '.') {
            return {};
        }

        fileName = fileNameStr;
    } catch (...) {
        throw HttpError(Pistache::Http::Code::Not_Found, "");
    }

    if (fileName.empty()) {
        throw HttpError(Pistache::Http::Code::Not_Found, "");
    }

    auto absStorage =
        std::filesystem::absolute(std::filesystem::canonical(storageDirectory));

    auto fullName = absStorage / fileName;

    try {
        fullName =
            std::filesystem::absolute(std::filesystem::canonical(fullName));
    } catch (...) {
        throw HttpError(Pistache::Http::Code::Not_Found, "");
    }

    if (!std::filesystem::exists(fullName)) {
        throw HttpError(Pistache::Http::Code::Not_Found, "");
    }

    if (fullName.string().find(absStorage) != 0) {
        throw HttpError(Pistache::Http::Code::Bad_Request, "");
    }

    return fullName;
}

// NOLINTNEXTLINE
void CStorageController::request_file(const Pistache::Rest::Request &request,
                                      ResponseWriter response) {
    std::filesystem::path fullName = getFilePath(request);

    if (fullName.empty()) {
        throw HttpError(Pistache::Http::Code::Not_Found, "");
    }

    std::filesystem::file_time_type lastWriteTime =
        std::filesystem::last_write_time(fullName);

    auto lastWriteTimeCTime =
        PistacheCustomHttpHeaders::LastModified::timeCast(lastWriteTime);
    auto lastWriteTimeGmt = (lastWriteTimeCTime + timezone);

    if (auto IfModifiedSince =
            request.headers().tryGetRaw("If-Modified-Since")) {
        auto cTimeIfModifiedSince =
            PistacheCustomHttpHeaders::LastModified::getTimeFromStr(
                IfModifiedSince->value());
        if (lastWriteTimeGmt <= cTimeIfModifiedSince) {
            response.send(Pistache::Http::Code::Not_Modified);
            return;
        }
    }

    response.headers().add(
        std::make_shared<PistacheCustomHttpHeaders::LastModified>(
            lastWriteTime));

    if (request.method() == Pistache::Http::Method::Get) {
        Pistache::Http::serveFile(response, fullName);
        return;
    }

    auto fileSize = std::filesystem::file_size(fullName);
    response.headers().add(
        std::make_shared<Pistache::Http::Header::ContentLength>(fileSize));

    auto stream = response.stream(Pistache::Http::Code::Ok);
    stream.ends();
}

void CStorageController::register_routes(const std::string &baseAddr,
                                         Pistache::Rest::Router &router) {
    std::error_code eCode;
    std::filesystem::create_directories(storageDirectory, eCode);
    Pistache::Rest::Routes::Get(
        router, baseAddr + "/storage/:fileName",
        Pistache::Rest::Routes::bind(&CStorageController::request_file, this));
    Pistache::Rest::Routes::Head(
        router, baseAddr + "/storage/:fileName",
        Pistache::Rest::Routes::bind(&CStorageController::request_file, this));
}

CStorageController::~CStorageController() = default;
