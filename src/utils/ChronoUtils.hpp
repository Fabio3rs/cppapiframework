#pragma once

#include <array>
#include <chrono>
#include <ctime>
#include <string>

class ChronoUtils {
  public:
    static auto
    GetDateAndTime(std::chrono::high_resolution_clock::time_point now =
                       std::chrono::high_resolution_clock::now())
        -> std::string {
        std::time_t tt = std::chrono::high_resolution_clock::to_time_t(now);

        auto mksec = std::chrono::duration_cast<std::chrono::microseconds>(
                         now.time_since_epoch())
                         .count();
        mksec %= 1000000;

        std::string str;

        {
            std::array<char, 32> buf{};

            size_t strft_res_sz =
                strftime(buf.data(), buf.size(), "%Y/%m/%d %H:%M:%S.",
                         std::localtime(&tt));

            str.reserve(28);
            str.append(buf.data(), strft_res_sz);
        }

        {
            std::string mksecstr = std::to_string(mksec);
            size_t mksecsz = mksecstr.size();

            if (mksecsz < 6) {
                str.append(6 - mksecsz, '0');
            }

            str += mksecstr;
        }

        return str;
    }
};
