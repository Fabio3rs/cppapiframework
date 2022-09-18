#pragma once

/*
 * @brief Test/dev code for websockets with pistache rest websever
 */

#include "../stdafx.hpp"
#include <memory>
#include <pistache/router.h>

class RouterHandlerProxy : public Pistache::Rest::Private::RouterHandler {

  public:
    /*void onRequest(const Pistache::Http::Request &request,
                   Pistache::Http::ResponseWriter response) override;*/

    void
    onConnection(const std::shared_ptr<Pistache::Tcp::Peer> &peer) override;
    void
    onDisconnection(const std::shared_ptr<Pistache::Tcp::Peer> &peer) override;
    void onInput(const char *buffer, size_t len,
                 const std::shared_ptr<Pistache::Tcp::Peer> &peer) override;

    explicit RouterHandlerProxy(Pistache::Rest::Router &router)
        : Pistache::Rest::Private::RouterHandler(router) {}

    [[nodiscard]] auto clone() const
        -> std::shared_ptr<Pistache::Tcp::Handler> override {
        return std::make_shared<RouterHandlerProxy>(*this);
    }

    RouterHandlerProxy(const RouterHandlerProxy &) = default;
    RouterHandlerProxy(RouterHandlerProxy &&) = default;
    auto operator=(const RouterHandlerProxy &)
        -> RouterHandlerProxy & = default;
    auto operator=(RouterHandlerProxy &&) -> RouterHandlerProxy & = default;
    ~RouterHandlerProxy() override;
};
