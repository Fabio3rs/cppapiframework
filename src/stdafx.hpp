#pragma once

#include "Database/GenericDBConnection.hpp"
#include "utils/CConfig.hpp"
#include <Poco/Base64Encoder.h>
#include <Poco/Crypto/DigestEngine.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/HMACEngine.h>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
#include <Poco/StreamCopier.h>
#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <pistache/http.h>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
