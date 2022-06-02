#include "../src/utils/NestedJson.hpp"
#include "../src/utils/PocoJsonStringify.hpp"
#include "allocation_count.hpp"
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <atomic>
#include <cstddef>
#include <gtest/gtest.h>
#include <iterator>
#include <new>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace {
std::string_view json = R"json({
            "Tx": {
                "OrgnlTx": {
                    "InitrTxId": "00000000000000000000",
                    "RcptTxId": "12345678900001"
                }
            }
        })json";

std::string_view jsonComArray = R"json({
            "Tx": {
                "SomeThing": [
                    {
                        "OrgnlTx": {
                            "InitrTxId": "00000000000000000000",
                            "RcptTxId": "12345678900000"
                        }
                    },
                    {
                        "OrgnlTx": {
                            "InitrTxId": "1234567891011123456",
                            "RcptTxId": "0000000000000",

                            "thing": [
                                10, 20, 30, 40
                            ]
                        }
                    }
                ]
            }
        })json";
} // namespace

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(JsonNestedGetValue, getnested) {
    using parser = Poco::JSON::Parser;
    using pObject_t = Poco::JSON::Object::Ptr;
    using namespace NestedJson;
    auto obj = parser().parse(std::string(json)).extract<pObject_t>();

    auto objCArray =
        parser().parse(std::string(jsonComArray)).extract<pObject_t>();

    EXPECT_EQ(getNestedValue<std::string>(obj, "Tx", "OrgnlTx", "RcptTxId"),
              "12345678900001");

    EXPECT_EQ(getArrObjNestedValue<std::string>(objCArray, "Tx", "SomeThing", 1,
                                                "OrgnlTx", "thing", 1),
              "20");
    EXPECT_EQ(getArrObjNestedValue<std::string>(objCArray, "Tx", "SomeThing", 1,
                                                "OrgnlTx", "thing", 1),
              "20");
    EXPECT_EQ(getArrObjNestedValue<int64_t>(objCArray, "Tx", "SomeThing", 1,
                                            "OrgnlTx", "thing", 1),
              20);
}
