/**
 * @file PocoJsonStringify.hpp
 * @brief Objetivo desse arquivo é prover funções de stringify de JSON da Poco
 * Library usando pouca ou nenhuma alocação em heap. Funções desse arquivo NÃO
 possuem funcionamento idêntico às originais da Poco Library.
 * This file objective is to provide stringify functions with low or maybe none
 heap allocations, this functions don't have the exactly behavior of the Poco
 Library original functions, some things are missing. In case of problem use the
 original functions.
 * @date 2022-01-30
 *
    Boost Software License - Version 1.0 - August 17th, 2003

    Permission is hereby granted, free of charge, to any person or organization
    obtaining a copy of the software and accompanying documentation covered by
    this license (the "Software") to use, reproduce, display, distribute,
    execute, and transmit the Software, and to prepare derivative works of the
    Software, and to permit third-parties to whom the Software is furnished to
    do so, all subject to the following:

    The copyright notices in the Software and this entire statement, including
    the above license grant, this restriction and the following disclaimer,
    must be included in all copies of the Software, in whole or in part, and
    all derivative works of the Software, unless such copies or derivative
    works are solely in the form of machine-executable object code generated by
    a source language processor.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
    SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
    FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.

    ---------------------------------------------------------------------------
    Note:
    Individual files contain the following tag instead of the full license text.

        SPDX-License-Identifier:	BSL-1.0

    This enables machine processing of license information based on the SPDX
    License Identifiers that are here available: http://spdx.org/licenses/
 *
 */
#pragma once
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <array>
#include <ostream>
#include <string_view>

class PocoJsonStringify {

  public:
    std::string str;
    int indent = 0, step = -1;
    bool strictJSON = true;
    bool escapeAllUnicode = false;

    /**
     * @brief função de stringify Poco::JSON::Object
     * https://github.com/pocoproject/poco/blob/master/JSON/include/Poco/JSON/Object.h
     *
     //
    // Object.h
    //
    // Library: JSON
    // Package: JSON
    // Module:  Object
    //
    // Definition of the Object class.
    //
    // Copyright (c) 2012, Applied Informatics Software Engineering GmbH.
    // and Contributors.
    //
    // SPDX-License-Identifier:	BSL-1.0
    //
     */
    void stringify(const Poco::JSON::Object &obj) {
        append('{');
        bool first = true;
        for (const auto &item : obj) {
            if (!first) {
                append(',');
            }
            first = false;
            formatString(item.first);
            append(":");
            stringify(item.second);
        }
        append('}');
    }

    void stringify(const Poco::JSON::Object::Ptr &arr) { stringify(*arr); }

    /**
     * @brief função de stringify baseada em Poco::JSON::Array
     * https://github.com/pocoproject/poco/blob/master/JSON/include/Poco/JSON/Array.h
     *
     //
    // Array.h
    //
    // Library: JSON
    // Package: JSON
    // Module:  Array
    //
    // Definition of the Array class.
    //
    // Copyright (c) 2012, Applied Informatics Software Engineering GmbH.
    // and Contributors.
    //
    // SPDX-License-Identifier:	BSL-1.0
    //
     */
    void stringify(const Poco::JSON::Array &arr) {
        append('[');
        bool first = true;
        for (const auto &item : arr) {
            if (!first) {
                append(',');
            }
            first = false;
            stringify(item);
        }
        append(']');
    }

    void stringify(const Poco::JSON::Array::Ptr &arr) { stringify(*arr); }

    /**
     * @brief função de stringify baseada em Poco::Dynamic::Var
     * https://github.com/pocoproject/poco/blob/master/JSON/src/Stringifier.cpp
     *
     //
    // Stringifier.cpp
    //
    // Library: JSON
    // Package: JSON
    // Module:  Stringifier
    //
    // Copyright (c) 2012, Applied Informatics Software Engineering GmbH.
    // and Contributors.
    //
    // SPDX-License-Identifier:	BSL-1.0
    //
     */
    void stringify(const Poco::Dynamic::Var &any) {
        using Object = Poco::JSON::Object;
        using Array = Poco::JSON::Array;

        const auto &type = any.type();
        if (type == typeid(Object)) {
            stringify(any.extract<Object>());
        } else if (type == typeid(Array)) {
            stringify(any.extract<Array>());
        } else if (type == typeid(Object::Ptr)) {
            stringify(*any.extract<Object::Ptr>());
        } else if (type == typeid(Array::Ptr)) {
            stringify(*any.extract<Array::Ptr>());
        } else if (any.isEmpty()) {
            append("null");
        } else if (any.isNumeric() || any.isBoolean()) {
            auto value = any.convert<std::string>();
            if (type == typeid(char)) {
                formatString(value);
            } else {
                append(value);
            }
        } else if (any.isString()) {
            formatString(any.extract<std::string>());
        } else if (any.isString() || any.isDateTime() || any.isDate() ||
                   any.isTime()) {
            formatString(any.convert<std::string>());
        } else {
            formatString(any.convert<std::string>());
        }
    }

    /**
     * @brief Format string para JSON, fonte base: Poco Library
     *
     //
    // String.h
    //
    // Library: Foundation
    // Package: Core
    // Module:  String
    //
    // Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
    // and Contributors.
    //
    // SPDX-License-Identifier:	BSL-1.0
    //
     */
    void formatString(const std::string &value) noexcept {
        append("\"");

        if (escapeAllUnicode) {
            escapeJSONUTF8(value.begin(), value.end());
        } else {
            for (std::string::const_iterator it = value.begin(),
                                             end = value.end();
                 it != end; ++it) {
                if ((*it >= 0 && *it <= 31) || (*it == '"') || (*it == '\\')) {
                    escapeJSONUTF8(it, it + 1);
                } else {
                    append(*it);
                }
            }
        }

        append("\"");
    }

    /**
     * @brief Escape string UTF8 JSON. Fonte: Poco Library
     * https://github.com/pocoproject/poco/blob/fac2437fabf24ff56df7ded9f468d3d855058221/Foundation/src/UTF8String.cpp#L181
     *
     //
    // UTF8String.cpp
    //
    // Library: Foundation
    // Package: Text
    // Module:  UTF8String
    //
    // Copyright (c) 2007, Applied Informatics Software Engineering GmbH.
    // and Contributors.
    //
    // SPDX-License-Identifier:	BSL-1.0
    //
     */
    void escapeJSONUTF8(const std::string::const_iterator &begin,
                        const std::string::const_iterator &end) noexcept {
        constexpr std::array<uint32_t, 6> offsetsFromUTF8{
            0x00000000UL, 0x00003080UL, 0x000E2080UL,
            0x03C82080UL, 0xFA082080UL, 0x82082080UL};

        std::string::const_iterator it = begin;

        while (it != end) {
            uint32_t ch = 0;
            unsigned int sz = 0;

            do {
                ch <<= 6U;
                ch += static_cast<unsigned char>(*it++);
                sz++;
            } while (it != end &&
                     (static_cast<unsigned char>(*it) & 0xC0U) == 0x80U &&
                     sz < 6);
            ch -= offsetsFromUTF8[sz - 1];

            if (ch == '\n') {
                append("\\n");
            } else if (ch == '\t') {
                append("\\t");
            } else if (ch == '\r') {
                append("\\r");
            } else if (ch == '\b') {
                append("\\b");
            } else if (ch == '\f') {
                append("\\f");
            } else if (ch == '\v') {
                append((strictJSON ? "\\u000B" : "\\v"));
            } else if (ch == '\a') {
                append((strictJSON ? "\\u0007" : "\\a"));
            } else if (ch == '\\') {
                append("\\\\");
            } else if (ch == '\"') {
                append("\\\"");
            } else if (ch == '/') {
                append("\\/");
            } else if (ch == '\0') {
                append("\\u0000");
            } else if (ch < 32 || ch == 0x7f) {
                append("\\u");
                std::string tmp;
                Poco::NumberFormatter::appendHex(
                    tmp, static_cast<unsigned char>(ch), 4);
                append(tmp);
            } else if (ch > 0xFFFF) {
                ch -= 0x10000;
                append("\\u");
                std::string tmp;
                Poco::NumberFormatter::appendHex(
                    tmp,
                    static_cast<unsigned char>((ch >> 10U) & 0x03ffU) + 0xd800U,
                    4);
                append(tmp);
                append("\\u");
                tmp.clear();
                Poco::NumberFormatter::appendHex(
                    tmp, static_cast<unsigned char>(ch & 0x03ffU) + 0xdc00, 4);
                append(tmp);
            } else if (ch >= 0x80 && ch <= 0xFFFF) {
                append("\\u");
                std::string tmp;
                Poco::NumberFormatter::appendHex(
                    tmp, static_cast<unsigned char>(ch), 4);
                append(tmp);
            } else {
                append(static_cast<char>(ch));
            }
        }
    }

    void append(unsigned char in) noexcept { str += static_cast<char>(in); }

    void append(char in) noexcept { str += in; }

    void append(std::string_view in) noexcept { str += in; }

    template <class input_t>
    void append(std::ostream &out, const input_t &in) noexcept {
        out << in;
    }
};
