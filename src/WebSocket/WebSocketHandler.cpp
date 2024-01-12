/*
 * @brief Test/dev code for websockets with pistache rest websever
 */
#include "../projstdafx.hpp"

// NOLINTNEXTLINE
auto WebSocketHandler::frame::receiveData(const char *buffer, size_t lenraw)
    -> std::pair<size_t, bool> {
    const auto *bytesRaw = reinterpret_cast<const uint8_t *>(buffer);
    size_t inBufferOffset = 0;

    bool done = false;

    while (inBufferOffset < lenraw && !done) {
        switch (readingState) {
        case 0:
            flags = bytesRaw[inBufferOffset++];
            ++readingState;
            break;

        case 1:
            lenByte = bytesRaw[inBufferOffset++];
            useMask = (lenByte & 0x80) != 0; // NOLINT
            size = lenByte & 0x7F;           // NOLINT
            ++readingState;
            bufferPos = 0;
            break;

        case 2:
            if (size == 126 || size == 127) {
                bufferTmp[bufferPos++] = buffer[inBufferOffset++];

                if (bufferPos == 8 && size == 127) {
                    size = be64toh(
                        *reinterpret_cast<uint64_t *>(bufferTmp.data()));
                    ++readingState;
                    bufferPos = 0;
                } else if (bufferPos == 2 && size == 126) {
                    size = be16toh(
                        *reinterpret_cast<uint16_t *>(bufferTmp.data()));
                    ++readingState;
                    bufferPos = 0;
                }
            } else {
                ++readingState;
                bufferPos = 0;
            }
            break;

        case 3:
            if (useMask) {
                bufferTmp[bufferPos++] = buffer[inBufferOffset++];

                if (bufferPos == 4) {
                    std::copy_n(bufferTmp.begin(), 4, mask.begin());
                    ++readingState;
                    bufferPos = 0;
                }
            } else {
                ++readingState;
            }
            break;
        case 4:
            payload.resize(size);
            ++readingState;
            bufferPos = 0;
            break;

        default:
            assert(size == payload.size()); // NOLINT
            assert(bufferPos < payload.size()); // NOLINT
            payload[bufferPos++] = buffer[inBufferOffset++];

            if (bufferPos == size) {
                done = true;
            }
            break;
        }
    }

    if (!done) {
        return {inBufferOffset, false};
    }

    readingState = 0;
    bufferPos = 0;

    if (useMask) {
        for (size_t i = 0; i < payload.size(); i++) {
            // NOLINTNEXTLINE
            payload[i] ^= mask[i % 4];
        }
    }

    return {inBufferOffset, true};
}

void WebSocketHandler::onInput(
    const char *buffer, size_t lenraw,
    const std::shared_ptr<Pistache::Tcp::Peer> &inpeer) {

    size_t inBufferOffset = 0;

    do {
        bool done = false;
        const char *bufferit = buffer + inBufferOffset;
        size_t currentLen = lenraw - inBufferOffset;

        std::tie(inBufferOffset, done) =
            frameInst.receiveData(bufferit, currentLen);

        if (!done) {
            return;
        }

        frameInst.peer = &inpeer;

        if (onMessage) {
            onMessage(frameInst);
        }

        frameInst.flags = 0;
        frameInst.lenByte = 0;
    } while (inBufferOffset < lenraw);
}
