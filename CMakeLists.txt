set(projectname cppapiframework)

cmake_minimum_required(VERSION 3.6.0)
project(${projectname} VERSION 0.1.0)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Werror -std=c++17 -Wno-c++98-compat -Wno-padded -Wno-date-time -Wno-c++98-compat-pedantic -Wno-exit-time-destructors -Wno-global-constructors")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_DEFAULT 17)
set(CXX17 ON)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_C_EXTENSIONS Off)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CTest)
enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

find_package(Poco REQUIRED COMPONENTS Crypto JSON Net NetSSL Redis)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig)
pkg_check_modules(Pistache REQUIRED IMPORTED_TARGET libpistache)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)

# setando todos arquivos fontes que serão compilados
file(
    GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/WebInterface/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/models/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Authorization/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Database/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/queues/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jobhandler/*.cpp"
)

add_library(${projectname} ${SOURCES})

target_compile_definitions(${projectname} PRIVATE "PROJECT_NAME=\"cppapiframework\"")

# Compiler-specific compile flags
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    message(STATUS "Setting G++ flags")
    # G++
    target_compile_options(${projectname} PRIVATE -Wall -Werror -Wextra -std=c++17 -Wformat-security -Wconversion -Wsign-conversion)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    message(STATUS "Setting MSVC flags")
    # MSVC
    target_compile_options(${projectname} PRIVATE /EHsc /W2 /c)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    message(STATUS "Setting Clang flags")
    target_compile_options(${projectname} PRIVATE -Weverything -Werror -Wno-date-time -Wno-disabled-macro-expansion -Wno-global-constructors -std=c++17 -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-exit-time-destructors)
    # Clang-tidy
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-header-filter=${CMAKE_CURRENT_SOURCE_DIR}/src;--warnings-as-errors=\"*\";-extra-arg=-std=c++17;-export-fixes;-checks=read*,modern*,hicpp*,performance*")
endif()

target_precompile_headers(${projectname} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.hpp")

# verifica se o poco foi encontrado e faz o link com a lib
if(Poco_FOUND)
    message("-- Lib Poco encontrada!")
    target_link_libraries(${projectname} ${Poco_LIBRARIES})
else()
    message(FATAL_ERROR "-- Biblioteca poco não encontrada")
endif(Poco_FOUND)

if(Pistache_FOUND)
    message("-- Pistache encontrado!")
    target_link_libraries(${projectname} PkgConfig::Pistache)
else()
    message(FATAL_ERROR "-- Pistache is not found in your system.")
endif(Pistache_FOUND)

target_link_libraries(${projectname} mysqlcppconn)


