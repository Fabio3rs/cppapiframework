#pragma once

#include "CLog.hpp"

#define CLOG_EXCEPTION_LOG(e)                                                  \
    CLog::log().multiRegister("EXCEPTION (%0) AT %1:%2", e.what(), __FILE__,   \
                              __LINE__)

#define CLOG_LOG(format, ...)                                                  \
    CLog::log().multiRegister("%1:%2 (%0) " format, __func__, __FILE__,        \
                              __LINE__, __VA_ARGS__)

