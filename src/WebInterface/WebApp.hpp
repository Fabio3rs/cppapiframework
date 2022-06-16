#pragma once

#include <functional>
#include <utility>

#include "httpwrappers.hpp"

namespace webapp {

using namespace httpwrappers;

class WebApp {
  public:
    auto getPort() const -> Pistache::Port {
        return httpEndpoint ? httpEndpoint->getPort() : Pistache::Port{0};
    }

    auto init(Address addr, size_t thr) -> WebApp & {
        httpEndpoint = std::make_shared<Pistache::Http::Endpoint>(addr);

        auto opts =
            Pistache::Http::Endpoint::options().threads(static_cast<int>(thr));
        opts.flags(Pistache::Tcp::Options::ReuseAddr);

        httpEndpoint->init(opts);

        return *this;
    }

    auto startAsync() -> WebApp & {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serveThreaded();

        return *this;
    }

    auto start(const std::function<void()> &callback) -> WebApp & {
        startAsync();

        if (callback) {
            callback();
        }

        while (keepRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        return *this;
    }

    auto stop() -> WebApp & {
        keepRunning = false;
        if (httpEndpoint) {
            httpEndpoint->shutdown();
        }

        return *this;
    }

    auto get_router() -> Pistache::Rest::Router & { return router; }

    template <class T>
    auto get(const std::string &resource, T func) -> WebApp & {
        router.get(resource, RouterWrapper(callback_fn_cast(func)));
        return *this;
    }

    template <class T>
    auto post(const std::string &resource, T func) -> WebApp & {
        router.post(resource, RouterWrapper(callback_fn_cast(func)));
        return *this;
    }

    template <class T>
    auto put(const std::string &resource, T func) -> WebApp & {
        router.put(resource, RouterWrapper(callback_fn_cast(func)));
        return *this;
    }

    template <class T>
    auto del(const std::string &resource, T func) -> WebApp & {
        router.del(resource, RouterWrapper(callback_fn_cast(func)));
        return *this;
    }

    template <class T>
    auto patch(const std::string &resource, T func) -> WebApp & {
        router.patch(resource, RouterWrapper(callback_fn_cast(func)));
        return *this;
    }

    template <class T>
    auto options(const std::string &resource, T func) -> WebApp & {
        router.options(resource, RouterWrapper(callback_fn_cast(func)));
        return *this;
    }

    template <class T>
    auto head(const std::string &resource, T func) -> WebApp & {
        router.head(resource, RouterWrapper(callback_fn_cast(func)));
        return *this;
    }

    template <class T> auto notfound(T func) -> WebApp & {
        router.addNotFoundHandler(RouterWrapper(callback_fn_cast(func)));
        return *this;
    }

    WebApp(const WebApp &) = delete;
    auto operator=(const WebApp &) -> WebApp & = delete;

    WebApp(const WebApp &&) = delete;
    auto operator=(const WebApp &&) -> WebApp & = delete;

    WebApp() = default;
    WebApp(Address addr, size_t thr) { init(addr, thr); }
    WebApp(uint16_t port, size_t thr) { init({Ipv4::any(), port}, thr); }
    ~WebApp();

  protected:
    void prepareReqResp(Req &req, Resp &resp);

    std::shared_ptr<Pistache::Http::Endpoint> httpEndpoint;
    Pistache::Rest::Router router;
    std::atomic<bool> keepRunning{true};
};

} // namespace webapp
