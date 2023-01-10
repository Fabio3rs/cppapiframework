#pragma once
#include <filesystem>
#ifndef CStorageController_hpp
#define CStorageController_hpp

#include "../WebInterface/CController.hpp"
#include "../stdafx.hpp"

class CStorageController : public CController {

  public:
    CStorageController() = default;
    CStorageController(const CStorageController &) = default;
    CStorageController(CStorageController &&) = delete;
    auto operator=(const CStorageController &) -> CStorageController & = delete;
    auto operator=(CStorageController &&) -> CStorageController & = delete;

    [[nodiscard]] auto getFilePath(const Pistache::Rest::Request &request) const
        -> std::filesystem::path;
    
    void request_file(const Pistache::Rest::Request &request,
                  Pistache::Http::ResponseWriter response);

    void register_routes(const std::string &baseAddr,
                         Pistache::Rest::Router &router) override;

    std::filesystem::path storageDirectory{"./storage/public"};

    ~CStorageController() override;
};

#endif
