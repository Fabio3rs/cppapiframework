#pragma once

#include "../stdafx.hpp"

class ScopedStreamRedirect {
    std::ostream &originalStream;
    std::streambuf *originalBuffer;

  public:
    ScopedStreamRedirect(const ScopedStreamRedirect &) = delete;
    auto operator=(const ScopedStreamRedirect &) -> ScopedStreamRedirect & = delete;
    ScopedStreamRedirect(ScopedStreamRedirect &&) = delete;
    auto operator=(ScopedStreamRedirect &&) -> ScopedStreamRedirect & = delete;

    inline ScopedStreamRedirect(std::ostream &srcStream,
                                std::ostream &destStream)
        : originalStream(srcStream),
          originalBuffer(srcStream.rdbuf(destStream.rdbuf())) {}

    inline ~ScopedStreamRedirect() { originalStream.rdbuf(originalBuffer); }
};
