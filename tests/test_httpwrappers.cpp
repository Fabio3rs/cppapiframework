#include "../src//WebInterface/WebApp.hpp"
#include "WebInterface/httpwrappers.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

using namespace webapp;

// NOLINTBEGIN(hicpp-special-member-functions)

static auto somefn(Req /*ununsed*/, Resp /*ununsed*/)
    -> std::shared_ptr<ResponseViaReturn> {
    return std::make_shared<RawStringResponse>("pato voa");
}

TEST(WebAppTest, Startup) {
    WebApp app({Ipv4::any(), 3000}, 2);

    app.get("/", somefn);

    app.startAsync();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_TRUE(true);
}

// NOLINTEND(hicpp-special-member-functions)
