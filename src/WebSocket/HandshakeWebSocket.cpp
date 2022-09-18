/*
 * @brief Test/dev code for websockets with pistache rest websever, using Poco
 * Library for hashing and base64 functions
 */

#include "HandshakeWebSocket.hpp"
#include <Poco/Base64Encoder.h>

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

    // NOLINTNEXTLINE(readability-identifier-length)
    int fd = response_.getPeer()->fd();

    const auto buf = buf_.buffer().data();
    // std::cout.write(buf.c_str(), static_cast<std::streamsize>(buf.size()));
    write(fd, buf.c_str(), buf.size());
    write(fd, os.str().c_str(), os.str().size());
}

const std::string WEBSOCKET_GUID("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
const std::string WEBSOCKET_VERSION("13");

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
