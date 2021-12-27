#pragma once

#include "../stdafx.hpp"

class ScopedStreamRedirect {
    std::ostream &originalStream;
    std::streambuf *originalBuffer;

    ScopedStreamRedirect(const ScopedStreamRedirect &) = delete;
    ScopedStreamRedirect &operator=(const ScopedStreamRedirect &) = delete;
    ScopedStreamRedirect(ScopedStreamRedirect &&) = delete;
    ScopedStreamRedirect &operator=(ScopedStreamRedirect &&) = delete;

  public:
    inline ScopedStreamRedirect(std::ostream &srcStream,
                                std::ostream &destStream)
        : originalStream(srcStream),
          originalBuffer(srcStream.rdbuf(destStream.rdbuf())) {}

    inline ~ScopedStreamRedirect() { originalStream.rdbuf(originalBuffer); }
};
