#include "CStorageController.hpp"
#include <filesystem>
#include <pistache/http_defs.h>
#include <string>
#include <string_view>
#include <system_error>

using namespace Pistache::Http;

auto CStorageController::getFilePath(
    const Pistache::Rest::Request &request) const -> std::filesystem::path {
    std::string fileName;
    try {
        fileName = request.param(":fileName").as<std::string>();
    } catch (...) {
        throw HttpError(Pistache::Http::Code::Not_Found, "");
    }

    if (fileName.empty()) {
        throw HttpError(Pistache::Http::Code::Not_Found, "");
    }

    std::filesystem::path fullName = storageDirectory / fileName;

    return fullName;
}

// NOLINTNEXTLINE
void CStorageController::GET_file(const Pistache::Rest::Request &request,
                                  ResponseWriter response) {
    std::filesystem::path fullName = getFilePath(request);

    response.send(Pistache::Http::Code::Not_Found, fullName);
}

// NOLINTNEXTLINE
void CStorageController::HEAD_file(const Pistache::Rest::Request &request,
                                   ResponseWriter response) {
    std::filesystem::path fullName = getFilePath(request);

    response.send(Pistache::Http::Code::Not_Found, fullName);
}

void CStorageController::register_routes(const std::string &baseAddr,
                                         Pistache::Rest::Router &router) {
    std::error_code eCode;
    std::filesystem::create_directories(storageDirectory, eCode);
    Pistache::Rest::Routes::Get(
        router, baseAddr + "/storage/:fileName",
        Pistache::Rest::Routes::bind(&CStorageController::GET_file, this));
    Pistache::Rest::Routes::Head(
        router, baseAddr + "/storage/:fileName",
        Pistache::Rest::Routes::bind(&CStorageController::HEAD_file, this));
}

CStorageController::~CStorageController() = default;
