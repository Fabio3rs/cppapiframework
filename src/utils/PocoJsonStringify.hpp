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

class PocoJsonStringify {

  public:
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
    static void stringify(const Poco::JSON::Object &obj, std::string &str,
                          int indent, int step = -1, int options = 0) {
        append(str, '{');
        bool first = true;
        for (const auto &item : obj) {
            if (!first) {
                append(str, ',');
            }
            first = false;
            formatString(item.first, str, options);
            append(str, ":");
            stringify(item.second, str, indent, step, options);
        }
        append(str, '}');
    }

    static void stringify(const Poco::JSON::Object::Ptr &arr, std::string &str,
                          int indent, int step = -1, int options = 0) {
        stringify(*arr, str, indent, step, options);
    }

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
    static void stringify(const Poco::JSON::Array &arr, std::string &str,
                          int indent, int step = -1, int options = 0) {
        append(str, '[');
        bool first = true;
        for (const auto &item : arr) {
            if (!first) {
                append(str, ',');
            }
            first = false;
            stringify(item, str, indent, step, options);
        }
        append(str, ']');
    }

    static void stringify(const Poco::JSON::Array::Ptr &arr, std::string &str,
                          int indent, int step = -1, int options = 0) {
        stringify(*arr, str, indent, step, options);
    }

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
    static void stringify(const Poco::Dynamic::Var &any, std::string &str,
                          int indent, int step = -1, int options = 0) {
        using Object = Poco::JSON::Object;
        using Array = Poco::JSON::Array;

        if (step == -1) {
            step = indent;
        }

        if (any.type() == typeid(Object)) {
            const auto &o = any.extract<Object>();
            stringify(o, str, indent == 0 ? 0 : indent, step);
        } else if (any.type() == typeid(Array)) {
            const auto &a = any.extract<Array>();
            stringify(a, str, indent == 0 ? 0 : indent, step);
        } else if (any.type() == typeid(Object::Ptr)) {
            const auto &o = any.extract<Object::Ptr>();
            stringify(*o, str, indent == 0 ? 0 : indent, step);
        } else if (any.type() == typeid(Array::Ptr)) {
            const auto &a = any.extract<Array::Ptr>();
            stringify(*a, str, indent == 0 ? 0 : indent, step);
        } else if (any.isEmpty()) {
            append(str, "null");
        } else if (any.isNumeric() || any.isBoolean()) {
            auto value = any.convert<std::string>();
            if (any.type() == typeid(char)) {
                formatString(value, str, options);
            } else {
                append(str, value);
            }
        } else if (any.isString()) {
            formatString(any.extract<std::string>(), str, options);
        } else if (any.isString() || any.isDateTime() || any.isDate() ||
                   any.isTime()) {
            auto value = any.convert<std::string>();
            formatString(value, str, options);
        } else {
            append(str, any.convert<std::string>());
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
    static void formatString(const std::string &value, std::string &str,
                             int /**/) {
        append(str, "\"");
        escapeJSONUTF8(value.begin(), value.end(), true, str);
        append(str, "\"");
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
    static void escapeJSONUTF8(const std::string::const_iterator &begin,
                               const std::string::const_iterator &end,
                               bool strictJSON, std::string &str) {
        static std::array<uint32_t, 6> offsetsFromUTF8{
            0x00000000UL, 0x00003080UL, 0x000E2080UL,
            0x03C82080UL, 0xFA082080UL, 0x82082080UL};

        std::string::const_iterator it = begin;

        while (it != end) {
            uint32_t ch = 0;
            unsigned int sz = 0;

            do {
                ch <<= 6;
                ch += static_cast<unsigned char>(*it++);
                sz++;
            } while (it != end && (*it & 0xC0) == 0x80 && sz < 6);
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
                Poco::NumberFormatter::appendHex(
                    tmp, static_cast<unsigned char>(ch), 4);
                append(str, tmp);
            } else if (ch > 0xFFFF) {
                ch -= 0x10000;
                append(str, "\\u");
                std::string tmp;
                Poco::NumberFormatter::appendHex(
                    tmp,
                    static_cast<unsigned char>((ch >> 10) & 0x03ff) + 0xd800,
                    4);
                append(str, tmp);
                append(str, "\\u");
                tmp.clear();
                Poco::NumberFormatter::appendHex(
                    tmp, static_cast<unsigned char>(ch & 0x03ff) + 0xdc00, 4);
                append(str, tmp);
            } else if (ch >= 0x80 && ch <= 0xFFFF) {
                append(str, "\\u");
                std::string tmp;
                Poco::NumberFormatter::appendHex(
                    tmp, static_cast<unsigned char>(ch), 4);
                append(str, tmp);
            } else {
                append(str, static_cast<char>(ch));
            }
        }
    }

    template <class input_t>
    static void append(std::string &out, const input_t &in) {
        out += in;
    }

    template <class input_t>
    static void append(std::ostream &out, const input_t &in) {
        out << in;
    }
};