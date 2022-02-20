#include "PocoJsonStringify.hpp"

void PocoJsonStringify::escapeJSONUTF8(const std::string::const_iterator &begin,
                                       const std::string::const_iterator &end,
                                       bool strictJSON, std::string &str) {
    static std::array<uint32_t, 6> offsetsFromUTF8{0x00000000UL, 0x00003080UL,
                                                   0x000E2080UL, 0x03C82080UL,
                                                   0xFA082080UL, 0x82082080UL};

    std::string::const_iterator it = begin;

    while (it != end) {
        uint32_t ch = 0;
        unsigned int sz = 0;

        do {
            ch <<= 6U;
            ch += static_cast<unsigned char>(*it++);
            sz++;
        } while (it != end &&
                 (static_cast<unsigned char>(*it) & 0xC0U) == 0x80U && sz < 6);
        ch -= offsetsFromUTF8[sz - 1];

        if (ch == '\n') {
            append(str, "\\n");
        } else if (ch == '\t') {
            append(str, "\\t");
        } else if (ch == '\r') {
            append(str, "\\r");
        } else if (ch == '\b') {
            append(str, "\\b");
        } else if (ch == '\f') {
            append(str, "\\f");
        } else if (ch == '\v') {
            append(str, (strictJSON ? "\\u000B" : "\\v"));
        } else if (ch == '\a') {
            append(str, (strictJSON ? "\\u0007" : "\\a"));
        } else if (ch == '\\') {
            append(str, "\\\\");
        } else if (ch == '\"') {
            append(str, "\\\"");
        } else if (ch == '/') {
            append(str, "\\/");
        } else if (ch == '\0') {
            append(str, "\\u0000");
        } else if (ch < 32 || ch == 0x7f) {
            append(str, "\\u");
            std::string tmp;
            Poco::NumberFormatter::appendHex(tmp,
                                             static_cast<unsigned char>(ch), 4);
            append(str, tmp);
        } else if (ch > 0xFFFF) {
            ch -= 0x10000;
            append(str, "\\u");
            std::string tmp;
            Poco::NumberFormatter::appendHex(
                tmp,
                static_cast<unsigned char>((ch >> 10U) & 0x03ffU) + 0xd800U, 4);
            append(str, tmp);
            append(str, "\\u");
            tmp.clear();
            Poco::NumberFormatter::appendHex(
                tmp, static_cast<unsigned char>(ch & 0x03ffU) + 0xdc00, 4);
            append(str, tmp);
        } else if (ch >= 0x80 && ch <= 0xFFFF) {
            append(str, "\\u");
            std::string tmp;
            Poco::NumberFormatter::appendHex(tmp,
                                             static_cast<unsigned char>(ch), 4);
            append(str, tmp);
        } else {
            append(str, static_cast<char>(ch));
        }
    }
}
