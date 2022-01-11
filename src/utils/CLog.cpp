#include "CLog.hpp"
#include <cstdio>
#include <ctime>
#include <exception>
#include <iostream>

#ifndef LOG_FILE_NAME_PATH
#define LOG_FILE_NAME_PATH "program.log"
#endif

#ifndef FULL_COMMIT_HASH
#define FULL_COMMIT_HASH "#"
#endif

auto CLog::log(const char *filepath) -> CLog & {
    static CLog Log(filepath != nullptr ? filepath : LOG_FILE_NAME_PATH);
    return Log;
}

CLog::CLog(const std::string &NameOfFile) {
    Finished = false;
    FileName = NameOfFile;
    std::cout << "Iniciando log em " << NameOfFile << std::endl;
    // LogFile.rdbuf()->pubsetbuf(nullptr, 0);
    LogFile.open(NameOfFile, std::ios::out | std::ios::app);

    if (!LogFile.good()) {
        throw std::runtime_error("Impossivel criar ou abrir o arquivo de log");
    }

    LogFile << "***********************************************************"
                   "******************\n";
    LogFile << std::string("* Program compilation date/time: ") + __DATE__ +
                   " " + __TIME__;
    LogFile << "\n*********************************************************"
                   "********************\n";
    LogFile << "Full commit hash: " FULL_COMMIT_HASH;
    LogFile << "\n*********************************************************"
                   "********************\n";
    LogFile << "***********************************************************"
                   "******************\n* Log started at: ";
    LogFile <<  GetDateAndTime();
    LogFile << "\n*********************************************************"
                   "********************\n\n";
}

CLog::~CLog() noexcept { FinishLog(); }

auto CLog::GetDateAndTime() -> std::string {
    auto now = std::chrono::high_resolution_clock::now();

    std::time_t tt = std::chrono::high_resolution_clock::to_time_t(now);

    auto mksec = std::chrono::duration_cast<std::chrono::microseconds>(
                     now.time_since_epoch())
                     .count();
    mksec %= 1000000;

    std::string str;

    {
        std::array<char, 32> buf{};

        size_t strft_res_sz = strftime(
            buf.data(), buf.size(), "%Y/%m/%d %H:%M:%S.", std::localtime(&tt));

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

void CLog::logjson(Poco::JSON::Object::Ptr jsonobj,
                   const std::string &extraid) {

    std::string Temp;
    Temp.reserve(extraid.size() + 32);
    Temp += GetDateAndTime();
    Temp += ": ";

    if (jsonobj.isNull()) {
        Temp += extraid;
        Temp += " JSON Object is NULL";
        Temp += "\n";

        std::lock_guard<std::mutex> lck(Logmtx);
        LogFile << Temp;
        LogFile.flush();
        return;
    }

    Temp += extraid;
    Temp += " ";
    LogFile << Temp;

    std::lock_guard<std::mutex> lck(Logmtx);
    jsonobj->stringify(LogFile);
    LogFile << "\n";
    
    LogFile.flush();
}

void CLog::AddToLog(const std::string &Text, const std::string &extraid) {
    std::string Temp;
    Temp.reserve(Text.size() + extraid.size() + 16);
    Temp += GetDateAndTime();
    Temp += ": ";
    Temp += extraid;
    Temp += " ";
    Temp += Text;
    Temp += "\n";

    std::lock_guard<std::mutex> lck(Logmtx);
    LogFile << Temp;
    LogFile.flush();
}

void CLog::FinishLog() {
    std::string LogContents;
    LogContents += "\n*********************************************************"
                   "********************\n* Log Finished at: ";
    LogContents += GetDateAndTime();
    LogContents += "\n*********************************************************"
                   "********************\n";
    LogFile.clear();


    std::lock_guard<std::mutex> lck(Logmtx);
    LogFile.write(LogContents.c_str(),
                  static_cast<std::streamsize>(LogContents.size()));
    LogFile.flush();
    Finished = true;
}

void CLog::SaveBuffer() { LogFile.flush(); }
