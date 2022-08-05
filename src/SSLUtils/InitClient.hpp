#pragma once
#include "../utils/CConfig.hpp"
#include <Poco/Net/ConsoleCertificateHandler.h>
#include <Poco/Net/KeyConsoleHandler.h>
#include <Poco/Net/Net.h>
#include <Poco/Net/NetSSL.h>
#include <Poco/Net/SSLManager.h>

namespace SSLUtils {
inline void initNetwork() {
    Poco::Net::initializeNetwork();
    Poco::Net::initializeSSL();
}

inline void initClient(bool loadDefaultCAs = true) {
    initNetwork();

    auto &conf = CConfig::config();

    const std::string customCaBundle = "CUSTOM_CA_BUNDLE";
    if (auto ca_certificates = conf.at(customCaBundle);
        !ca_certificates.empty()) {
        using namespace Poco;
        using namespace Poco::Net;
        using PrivateKeyPassHandlerPtr = SharedPtr<PrivateKeyPassphraseHandler>;
        using InvalidCertHandlerPtr = SharedPtr<InvalidCertificateHandler>;

        PrivateKeyPassHandlerPtr pConsoleHandler = new KeyConsoleHandler(false);
        InvalidCertHandlerPtr pInvalidCertHandler =
            new ConsoleCertificateHandler(false);
        Context::Ptr pContext =
            new Context(Context::CLIENT_USE, "", "", ca_certificates,
                        Context::VERIFY_RELAXED, 9, loadDefaultCAs,
                        "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
        SSLManager::instance().initializeClient(pConsoleHandler,
                                                pInvalidCertHandler, pContext);
    }
}
} // namespace SSLUtils
