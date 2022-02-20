#pragma once

#include "../stdafx.hpp"
#include "BorrowPool.hpp"
#include "primitivepairhash.hpp"

#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/URI.h>

class CHttpPool {
    using inst_t = std::shared_ptr<Poco::Net::HTTPClientSession>;
    using pool_t = BorrowPool<inst_t>;
    using sessionid_t = std::pair<std::string, unsigned int>;

    std::mutex sessionmtx;
    std::unordered_map<sessionid_t, pool_t, primitivepairhash> sessions;

  public:
    auto setupSession(const Poco::URI &uri) -> BorrowedObject<inst_t>;

    static auto default_inst() -> CHttpPool &;

    CHttpPool() = default;
};
