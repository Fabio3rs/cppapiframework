#pragma once

#include "../src/stdafx.hpp"
#include "../src/JSON/StructParser.hpp"

namespace Json2StructTest {


struct AnotherStruct {
    std::string name;
    std::string lastname;
};

struct TestJs {
    std::string val;
    std::string anotherval;

    std::vector<std::string> dataarr;

    int64_t intvalue;

    AnotherStruct userdata;

    std::vector<AnotherStruct> otherUsers;

    std::unordered_map<std::string, AnotherStruct> multikeyvalue;

    bool algumacoisa{};
};

struct Helper {

static auto makeParserForTestJs(TestJs &) -> Poco::SharedPtr<JSONStructParser::StructParser>;

};

}

