#include "Json2StructTest.hpp"
#include "allocation_count.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <new>
#include <sstream>

namespace {
auto jsondata = R"json({
            "anotherval" : 0,
            "val" : "AAAAA TESTE",
            "dataarr": [
                "a",
                "b",
                "c",
                "d"
            ],
            "intvalue": 10,
            "userdata" : {
                "name" : "Fulano",
                "lastname" : "De Tal"
            },
            "otherUsers" : [
                {
                    "name" : "Fulano",
                    "lastname" : "De Tal 2"
                },
                {
                    "name" : "Fulano",
                    "lastname" : "De Tal 3"
                },
                {
                    "name" : "Fulano",
                    "lastname" : "De Tal 4"
                },
                {
                    "name" : "Fulano",
                    "lastname" : "De Tal 5"
                }
            ],
            "algumacoisa": true,
            "multikeyvalue": {
                "name" : {
                    "name" : "Fulano",
                    "lastname" : "De Tal 5"
                },
                "lastname" : {
                    "name" : "Fulano",
                    "lastname" : "De Tal 4"
                }
            }
        })json";

void parseJsonToStruct(Json2StructTest::TestJs &data) {
    Poco::JSON::Parser parser(
        Json2StructTest::Helper::makeParserForTestJs(data));
    parser.parse(jsondata);
}
} // namespace

// NOLINTNEXTLINE
TEST(Json2StructT, NormalParse) {
    Poco::JSON::Parser parser;
    parser.parse(jsondata);

    std::cout << "getAllocationCount " << AllocationCount::getAllocationCount()
              << std::endl;
    std::cout << "getDeallocationCount "
              << AllocationCount::getDeallocationCount() << std::endl;

    //EXPECT_TRUE(false);
}

// NOLINTNEXTLINE
TEST(Json2StructT, None) {
    Json2StructTest::TestJs data;
    parseJsonToStruct(data);

    std::cout << "getAllocationCount " << AllocationCount::getAllocationCount()
              << std::endl;
    std::cout << "getDeallocationCount "
              << AllocationCount::getDeallocationCount() << std::endl;

    //EXPECT_TRUE(false);

    std::cout << "data.anotherval " << data.anotherval << std::endl;
    std::cout << "data.val " << data.val << std::endl;
    std::cout << "data.intvalue " << data.intvalue << std::endl;
    std::cout << "data.userdata.name " << data.userdata.name << std::endl;
    std::cout << "data.userdata.lastname " << data.userdata.lastname
              << std::endl;

    for (const auto &d : data.dataarr) {
        std::cout << " dataarr val " << d << std::endl;
    }
    for (const auto &d : data.otherUsers) {
        std::cout << "d.name " << d.name << std::endl;
        std::cout << "d.lastname " << d.lastname << std::endl;
    }
    for (const auto &d : data.multikeyvalue) {
        std::cout << " multikeyvalue val " << d.first << "   " << d.second.name
                  << "    " << d.second.lastname << std::endl;
    }

    std::cout << "data.algumacoisa " << data.algumacoisa << std::endl;
}
