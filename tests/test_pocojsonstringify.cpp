#include "projstdafx.hpp"

#include "allocation_count.hpp"
#include <gtest/gtest.h>
#include <new>
#include <sstream>

static auto setupJson() {
    Poco::JSON::Object::Ptr testeptr(new Poco::JSON::Object);
    Poco::JSON::Object::Ptr JsonObject(new Poco::JSON::Object);
    JsonObject->set("JsonObject", "2562255184661247");

    testeptr->set("PlainTextInfo", JsonObject);

    Poco::JSON::Array::Ptr JsonObjectARR(new Poco::JSON::Array);
    for (int i = 0; i < 20; i++) {
        Poco::JSON::Object::Ptr usr(new Poco::JSON::Object);
        usr->set("id", "2562255184661247200000000000000");
        usr->set("username", "252255184661247");
        usr->set("name", "aaa\n");
        usr->set("test", "çã\n\"");

        JsonObjectARR->add(usr);
    }

    JsonObject->set("AAAAA", JsonObjectARR);

    return JsonObject;
}

// NOLINTNEXTLINE
TEST(PocoJSONStringify, StringifyTeste) {
    auto json = setupJson();

    EXPECT_GT(AllocationCount::getAllocationCount(), 0);
    AllocationCount::getAllocationCount() = 0;

    PocoJsonStringify stringifier;
    stringifier.str.reserve(4096);

    EXPECT_GT(AllocationCount::getAllocationCount(), 0);
    AllocationCount::getAllocationCount() = 0;

    stringifier.stringify(json);
    EXPECT_EQ(AllocationCount::getAllocationCount(), 0);

    std::stringstream sstr;
    json->stringify(sstr);

    EXPECT_EQ(stringifier.str, sstr.str());
}

// NOLINTNEXTLINE
TEST(PocoJSONStringify, StringifyNull) {
    auto json = setupJson();

    json->set("nullobject", Poco::JSON::Object::Ptr());
    json->set("nullarray", Poco::JSON::Array::Ptr());

    auto reparsedJson = Validator::parse_json_from_string(
        PocoJsonStringify::JsonToString(json));

    EXPECT_TRUE(reparsedJson->get("nullobject").isEmpty());
    EXPECT_TRUE(reparsedJson->get("nullarray").isEmpty());
}
