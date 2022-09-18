#pragma once

/*
 * @brief Test/dev code for websockets with pistache rest websever
 */

#include "../stdafx.hpp"
#include <pistache/peer.h>

class WebSocketHandler {
  public:
    struct frame {
        int32_t flags{}, lenByte{};
        uint64_t size{};
        std::array<int8_t, 4> mask{};
        int32_t readingState{};

        std::string payload{};
        size_t bufferPos{};
        std::array<char, 16> bufferTmp{};

        const std::shared_ptr<Pistache::Tcp::Peer> *peer{};

        bool useMask{};

        auto receiveData(const char *buffer, size_t lenraw)
            -> std::pair<size_t, bool>;
    };

    std::atomic<bool> disconnected{false};

  protected:
    frame frameInst;

  public:
    std::function<void(const frame &)> onMessage;
    std::function<void(const WebSocketHandler &,
                       const std::shared_ptr<Pistache::Tcp::Peer> &peer)>
        onDisconnection;
    std::weak_ptr<Pistache::Tcp::Peer> peer;

    void onInput(const char *buffer, size_t len,
                 const std::shared_ptr<Pistache::Tcp::Peer> &peer);
};
