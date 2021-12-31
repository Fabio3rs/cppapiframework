#include "CHttpPool.hpp"

CHttpPool &CHttpPool::default_inst() {
    static CHttpPool pool;
    return pool;
}

BorrowedObject<CHttpPool::inst_t>
CHttpPool::setupSession(const Poco::URI &uri) {
    sessionid_t session_id_map{uri.getHost(), uri.getPort()};

    auto poolinst = sessions.try_emplace(session_id_map, 16u);

    if (poolinst.first == sessions.end()) {
        throw std::runtime_error("Falha para receber o objeto do map");
    }

    pool_t &poolInstance = poolinst.first->second;
    auto session_inst = poolInstance.borrow();

    if (!session_inst) {
        return session_inst;
    }

    inst_t &session = *session_inst;

    if (!session) {
        if (uri.getScheme() == "https") {
            session = std::make_shared<Poco::Net::HTTPSClientSession>(
                uri.getHost(), uri.getPort());
        } else {
            session = std::make_shared<Poco::Net::HTTPClientSession>(
                uri.getHost(), uri.getPort());
        }

        session->setKeepAlive(true);
        session->setKeepAliveTimeout(Poco::Timespan(600L, 0L));
    }

    return session_inst;
}
