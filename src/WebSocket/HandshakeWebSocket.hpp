#pragma once

/*
 * @brief Test/dev code for websockets with pistache rest websever
 */

#include "../stdafx.hpp"
#include <Poco/SHA1Engine.h>
#include <pistache/http.h>
#include <pistache/peer.h>
#include <pistache/router.h>

namespace WebSocket {

class DisableParserStep : public Pistache::Http::Private::Step {
  public:
    // NOLINTNEXTLINE
    static constexpr Pistache::Http::Private::StepId Id =
        Pistache::Meta::Hash::fnv1a("DisableParserStep");

    explicit DisableParserStep(Pistache::Http::Request *request)
        : Step(request) {}

    explicit DisableParserStep(Pistache::Http::Private::Step &step)
        : Step(step) {}

    [[nodiscard]] auto id() const -> Pistache::Http::Private::StepId override {
        return Id;
    }

    auto apply(Pistache::StreamCursor & /*cursor*/)
        -> Pistache::Http::Private::State override {
        return Pistache::Http::Private::State::Again;
    }

    DisableParserStep(const DisableParserStep &) = default;
    auto operator=(const DisableParserStep &) -> DisableParserStep & = default;

    DisableParserStep(DisableParserStep &&) = default;
    auto operator=(DisableParserStep &&) -> DisableParserStep & = default;

    ~DisableParserStep() override;
};

/*
 * @brief this workaround class is for reconfigure the original pistache's http
 * parser after the handshake for upgrade connection
 */
class ParserChanger : public Pistache::Http::RequestParser {
  public:
    void disableParserStep() {
        reset();

        allSteps[0] = std::make_unique<DisableParserStep>(*allSteps[0]);
        currentStep = 0;
    }

    ParserChanger(const ParserChanger &) = delete;
    auto operator=(const ParserChanger &) -> ParserChanger & = delete;
    ParserChanger(ParserChanger &&) = default;
    auto operator=(ParserChanger &&) -> ParserChanger & = default;

    explicit ParserChanger(Pistache::Http::RequestParser &&other);

    ~ParserChanger() override;
};

class HandshakeWebSocket {

  public:
    /*
     * @brief this is a workaround
     */
    static void
    tweakDefaultHttpParser(const std::shared_ptr<Pistache::Tcp::Peer> &peer) {
        auto parser = Pistache::Http::Handler::getParser(peer);

        parser->reset();
        ParserChanger tmp(std::move(*parser));

        tmp.disableParserStep();
        *parser = std::move(tmp);
    }

    static void doAccept(const Pistache::Rest::Request &request,
                         Pistache::Http::ResponseWriter &response);
};

} // namespace WebSocket
