#include "../src/utils/PocoJsonStringify.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <new>
#include <sstream>

static std::atomic<int64_t> allocations{0};
static std::atomic<int64_t> deallocations{0};

auto operator new(std::size_t n) -> void* {
    ++allocations;
    return malloc(n); // NOLINT(hicpp-no-malloc)
}
void operator delete(void *p) noexcept {
    ++deallocations;
    free(p); // NOLINT(hicpp-no-malloc)
}

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

        JsonObjectARR->add(usr);
    }

    JsonObject->set("AAAAA", JsonObjectARR);

    return JsonObject;
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(PocoJSONStringify, StringifyTeste) {
    auto json = setupJson();

    EXPECT_GT(allocations, 0);
    allocations = 0;

    std::string outjson;
    outjson.reserve(4096);

    EXPECT_GT(allocations, 0);
    allocations = 0;

    PocoJsonStringify::stringify(json, outjson, 0);
    EXPECT_EQ(allocations, 0);

    std::stringstream sstr;
    json->stringify(sstr);

    EXPECT_EQ(outjson, sstr.str());
}
