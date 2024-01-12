#include "projstdafx.hpp"

#include <gtest/gtest.h>

namespace {
constexpr std::array DATA_TWO_WS_FRM{'\0', '\x01', 'A', '\0', '\x02', 'A', 'B'};
}

// NOLINTNEXTLINE(hicpp-special-member-functions)
TEST(TestWsInput, TestParseBaseMsg) {
    WebSocketHandler::frame frm;
    auto receiveState =
        frm.receiveData(DATA_TWO_WS_FRM.data(), DATA_TWO_WS_FRM.size());

    EXPECT_EQ(receiveState.first, 3);
    EXPECT_EQ(frm.payload, "A");
    EXPECT_TRUE(receiveState.second);

    const char *bufferit = DATA_TWO_WS_FRM.data() + receiveState.first;

    receiveState =
        frm.receiveData(bufferit, DATA_TWO_WS_FRM.size() - receiveState.first);

    EXPECT_EQ(receiveState.first, 4);
    EXPECT_EQ(frm.payload, "AB");
    EXPECT_TRUE(receiveState.second);
}

// NOLINTNEXTLINE
TEST(TestWsInput, TestHandler) {
    WebSocketHandler handler;

    size_t receivedFrames = 0;

    handler.onMessage = [&](const WebSocketHandler::frame &frame) {
        switch (receivedFrames++) {
        case 0: {
            EXPECT_EQ(frame.payload, "A");
            break;
        }

        case 1: {
            EXPECT_EQ(frame.payload, "AB");
            break;
        }

        default:
            EXPECT_TRUE(false) << receivedFrames;
            break;
        }
    };

    handler.onInput(DATA_TWO_WS_FRM.data(), DATA_TWO_WS_FRM.size(), {});
    EXPECT_EQ(receivedFrames, 2);
}
