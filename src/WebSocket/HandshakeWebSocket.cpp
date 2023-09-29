/*
 * @brief Test/dev code for websockets with pistache rest websever, using Poco
 * Library for hashing and base64 functions
 */

#include "HandshakeWebSocket.hpp"
#include <Poco/Base64Encoder.h>
#include <cassert>
#include <openssl/ssl.h>

namespace WebSocket {
DisableParserStep::~DisableParserStep() = default;

ParserChanger::ParserChanger(Pistache::Http::RequestParser &&other)
    : Pistache::Http::Private::ParserImpl<Pistache::Http::Request>(
          std::move(other)) {}

ParserChanger::~ParserChanger() = default;
} // namespace WebSocket

using namespace Pistache::Http;
using namespace Pistache;
namespace {
// https://github.com/pistacheio/pistache/blob/ec6508e0288e253b6b9081411461c93688b30f70/src/common/http.cc#L51
auto writeStatusLine(Version version, Code code, DynamicStreamBuf &buf)
    -> bool {
#define OUT(...)                                                               \
    do {                                                                       \
        __VA_ARGS__;                                                           \
        if (!os)                                                               \
            return false;                                                      \
    } while (0)

    // NOLINTNEXTLINE(readability-identifier-length)
    std::ostream os(&buf);

    OUT(os << version << " ");
    OUT(os << static_cast<int>(code));
    OUT(os << ' ');
    OUT(os << code);
    OUT(os << crlf);

    return true;

#undef OUT
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
auto writeHeaders(const Header::Collection &headers, DynamicStreamBuf &buf)
    -> bool {
#define OUT(...)                                                               \
    do {                                                                       \
        __VA_ARGS__;                                                           \
        if (!os)                                                               \
            return false;                                                      \
    } while (0)

    // NOLINTNEXTLINE(readability-identifier-length)
    std::ostream os(&buf);

    for (const auto &header : headers.list()) {
        OUT(os << header->name() << ": ");
        OUT(header->write(os));
        OUT(os << crlf);
    }

    for (const auto &header : headers.rawList()) {
        OUT(os << header.second.name() << ": ");
        OUT(os << header.second.value());
        OUT(os << crlf);
    }

    return true;

#undef OUT
}

auto writeCookies(const CookieJar &cookies, DynamicStreamBuf &buf) -> bool {
#define OUT(...)                                                               \
    do {                                                                       \
        __VA_ARGS__;                                                           \
        if (!os)                                                               \
            return false;                                                      \
    } while (0)

    // NOLINTNEXTLINE(readability-identifier-length)
    std::ostream os(&buf);
    for (const auto &cookie : cookies) {
        OUT(os << "Set-Cookie: ");
        OUT(os << cookie);
        OUT(os << crlf);
    }

    return true;

#undef OUT
}

inline auto pistachePeerWrite(Pistache::Tcp::Peer &peer, const void *buffer,
                              size_t len) -> ssize_t {
#ifdef PISTACHE_USE_SSL
    if (peer.ssl() != nullptr) {
        auto *ssl_ = static_cast<SSL *>(peer.ssl());
        return SSL_write(ssl_, buffer, static_cast<int>(len));
    }
#endif /* PISTACHE_USE_SSL */
    return ::write(peer.fd(), buffer, len);
}

inline auto streamWrite(Pistache::Tcp::Peer &peer, const void *buf, size_t n)
    -> ssize_t {
    ssize_t result = pistachePeerWrite(peer, buf, n);
    if (result < 0) {
        return result;
    }

    do {
        auto written = static_cast<size_t>(result);
        size_t diff = n - written;

        if (diff == 0) {
            return result;
        }

        ssize_t current = pistachePeerWrite(
            peer, reinterpret_cast<const uint8_t *>(buf) + written,
            n - written);

        if (current < 0) {
            return -result;
        }

        result += current;
    } while (true);
}

void putOnWire(Pistache::Http::ResponseWriter &response_) {
    Pistache::DynamicStreamBuf buf_(0, 256);
    // NOLINTNEXTLINE(readability-identifier-length)
    std::stringstream os;

    //(writeStatusLine(Version::Http11, response_.getResponseCode(), buf_));
    writeStatusLine(Version::Http11, Code::Switching_Protocols, buf_);
    writeHeaders(response_.headers(), buf_);
    (writeCookies(response_.cookies(), buf_));

    // OUT(writeHeader<Header::Connection>(os,
    // ConnectionControl::KeepAlive));

    os << crlf;

    auto peer = response_.getPeer();

    assert(peer); // NOLINT

    const auto buf = buf_.buffer().data();
    // std::cout.write(buf.c_str(), static_cast<std::streamsize>(buf.size()));
    streamWrite(*peer, buf.c_str(), buf.size());
    streamWrite(*peer, os.str().c_str(), os.str().size());
}

const std::string WEBSOCKET_GUID("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
[[maybe_unused]] const std::string WEBSOCKET_VERSION("13");

// https://github.com/pocoproject/poco/blob/devel/Net/src/WebSocket.cpp#L301
auto computeAccept(const std::string &key) -> std::string {
    std::string accept(key);
    accept += WEBSOCKET_GUID;
    Poco::SHA1Engine sha1;
    sha1.update(accept);
    Poco::DigestEngine::Digest dhash = sha1.digest();

    std::ostringstream ostr;

    {
        Poco::Base64Encoder base64(ostr);
        base64.write(reinterpret_cast<const char *>(dhash.data()),
                     static_cast<std::streamsize>(dhash.size()));
        base64.close();
    }

    return ostr.str();
}
} // namespace

void WebSocket::HandshakeWebSocket::doAccept(
    const Pistache::Rest::Request &request,
    Pistache::Http::ResponseWriter &response) {
    response.headers().addRaw(Header::Raw{"Upgrade", "websocket"});
    response.headers().addRaw(Header::Raw{"Connection", "Upgrade"});
    response.headers().addRaw(Header::Raw{
        "Sec-WebSocket-Accept",
        computeAccept(request.headers().getRaw("Sec-WebSocket-Key").value())});

    putOnWire(response);
}
