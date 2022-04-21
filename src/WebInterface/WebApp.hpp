#pragma once

#include <utility>

#include "httpwrappers.hpp"

namespace webapp {

using namespace httpwrappers;

class WebApp {
  public:
    auto getPort() const -> Pistache::Port {
        return httpEndpoint ? httpEndpoint->getPort() : Pistache::Port{0};
    }

    void init(Address addr, size_t thr) {
        httpEndpoint = std::make_shared<Pistache::Http::Endpoint>(addr);

        auto opts =
            Pistache::Http::Endpoint::options().threads(static_cast<int>(thr));
        opts.flags(Pistache::Tcp::Options::ReuseAddr);

        httpEndpoint->init(opts);
    }

    void startAsync() {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serveThreaded();
    }

    void start(const std::function<void()> &callback) {
        startAsync();

        if (callback) {
            callback();
        }

        while (keepRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    void stop() {
        keepRunning = false;
        if (httpEndpoint) {
            httpEndpoint->shutdown();
        }
    }

    auto get_router() -> Pistache::Rest::Router & { return router; }

    using callbackfn_t = std::function<RouterWrapper::callbackDecl_t>;

    auto get(const std::string &resource, callbackfn_t func) -> WebApp & {
        router.get(resource, RouterWrapper(std::move(func)));
        return *this;
    }

    auto post(const std::string &resource, callbackfn_t func) -> WebApp & {
        router.post(resource, RouterWrapper(std::move(func)));
        return *this;
    }

    auto put(const std::string &resource, callbackfn_t func) -> WebApp & {
        router.put(resource, RouterWrapper(std::move(func)));
        return *this;
    }

    auto del(const std::string &resource, callbackfn_t func) -> WebApp & {
        router.del(resource, RouterWrapper(std::move(func)));
        return *this;
    }

    auto patch(const std::string &resource, callbackfn_t func) -> WebApp & {
        router.patch(resource, RouterWrapper(std::move(func)));
        return *this;
    }

    auto options(const std::string &resource, callbackfn_t func) -> WebApp & {
        router.options(resource, RouterWrapper(std::move(func)));
        return *this;
    }

    auto head(const std::string &resource, callbackfn_t func) -> WebApp & {
        router.head(resource, RouterWrapper(std::move(func)));
        return *this;
    }

    auto notfound(callbackfn_t func) -> WebApp & {
        router.addNotFoundHandler(RouterWrapper(std::move(func)));
        return *this;
    }

    WebApp(const WebApp &) = delete;
    auto operator=(const WebApp &) -> WebApp & = delete;

    WebApp(const WebApp &&) = delete;
    auto operator=(const WebApp &&) -> WebApp & = delete;

    WebApp() = default;
    WebApp(Address addr, size_t thr) { init(addr, thr); }
    ~WebApp();

  private:
    void prepareReqResp(Req &req, Resp &resp);

    std::shared_ptr<Pistache::Http::Endpoint> httpEndpoint;
    Pistache::Rest::Router router;
    std::atomic<bool> keepRunning{true};
};

} // namespace webapp
