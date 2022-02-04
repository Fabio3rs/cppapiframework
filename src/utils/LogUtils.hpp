#pragma once

#include "CLog.hpp"

#ifndef CLOG_LOG

#define CLOG_EXCEPTION_LOG(e)                                                  \
    CLog::log().multiRegister("EXCEPTION (%0) AT %1:%2", e.what(), __FILE__,   \
                              __LINE__)

#define CLOG_LOG(format, ...)                                                  \
    CLog::log().multiRegister("%1:%2 (%0) " format, __func__, __FILE__,        \
                              __LINE__, __VA_ARGS__)

#define LOG_LN(level, format, ...)                                             \
    CLog::log().multiRegisterLN(__FILE__, __LINE__, level, format, __func__,   \
                                __VA_ARGS__)

#define LOGERR(format, ...) LOG_LN("E", __VA_ARGS__)
#define LOGWRN(format, ...) LOG_LN("W", __VA_ARGS__)
#define LOGINF(format, ...) LOG_LN("I", __VA_ARGS__)
#define LOGDBG(format, ...) LOG_LN("D", __VA_ARGS__)
#define LOGVRB(format, ...) LOG_LN("V", __VA_ARGS__)

#endif
