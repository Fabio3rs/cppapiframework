/*
 * @brief Test/dev code for websockets with pistache rest websever
 */

#include "RouterHandler.hpp"
#include "../stdafx.hpp"
#include "../utils/LogUtils.hpp"
#include "WebSocketHandler.hpp"
#include <cstddef>
#include <cstdint>
#include <exception>
#include <memory>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <utility>

void RouterHandlerProxy::onConnection(
    const std::shared_ptr<Pistache::Tcp::Peer> &peer) {
    // std::cout << __func__ << ": " << peer->fd() << std::endl;
    peer->putData(ParserData, std::make_shared<Pistache::Http::RequestParser>(
                                  getMaxRequestSize()));
}

void RouterHandlerProxy::onDisconnection(
    const std::shared_ptr<Pistache::Tcp::Peer> &peer) {
    LOGVRB("Disconnected", "");
    if (auto WSHandler = peer->tryGetData("__WEBSOCKETHANDLER")) {
        auto handler = std::static_pointer_cast<WebSocketHandler>(WSHandler);
        std::cout << "Websocket disconnected" << std::endl;

        try {
            if (handler->onDisconnection) {
                handler->onDisconnection(*handler, peer);
            }
        } catch (...) {
        }
    }

    Pistache::Rest::Private::RouterHandler::onDisconnection(peer);
}

void RouterHandlerProxy::onInput(
    const char *buffer, size_t len,
    const std::shared_ptr<Pistache::Tcp::Peer> &peer) {
    auto parser = getParser(peer);

    if (auto WSHandler = peer->tryGetData("__WEBSOCKETHANDLER")) {
        peer->setIdle(false);
        auto handler = std::static_pointer_cast<WebSocketHandler>(WSHandler);
        handler->onInput(buffer, len, peer);
        return;
    }

    using namespace Pistache::Http;
    auto &request = parser->request;
    try {
        if (!parser->feed(buffer, len)) {
            parser->reset();
            throw HttpError(Code::Request_Entity_Too_Large,
                            "Request exceeded maximum buffer size");
        }

        auto state = parser->parse();

        if (state == Private::State::Done) {
            ResponseWriter response(request.version(), transport(), this, peer);

#ifdef LIBSTDCPP_SMARTPTR_LOCK_FIXME
            request.associatePeer(peer);
#endif

            request.copyAddress(peer->address());

            auto connection = request.headers().tryGet<Header::Connection>();

            if (connection) {
                response.headers().add<Header::Connection>(
                    connection->control());
            } else {
                response.headers().add<Header::Connection>(
                    ConnectionControl::Close);
            }

            onRequest(request, std::move(response));
            parser->reset();
        }
    } catch (const HttpError &err) {
        ResponseWriter response(request.version(), transport(), this, peer);
        response.send(static_cast<Code>(err.code()), err.reason());
        parser->reset();
    }

    catch (const std::exception &e) {
        ResponseWriter response(request.version(), transport(), this, peer);
        response.send(Code::Internal_Server_Error, e.what());
        parser->reset();
    }
}

RouterHandlerProxy::~RouterHandlerProxy() = default;
