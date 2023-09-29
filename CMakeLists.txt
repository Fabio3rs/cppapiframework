set(projectname cppapiframework)

cmake_minimum_required(VERSION 3.6.0)
project(${projectname} VERSION 0.1.0)

if (NOT DEFINED CPPAPIFRAMEWORK_APPLY_FLAGS)
    message("Applying flags")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_DEFAULT 20)
    set(CXX17 ON)
    set(CMAKE_CXX_EXTENSIONS ON)
    set(CMAKE_C_EXTENSIONS ON)
endif()

option(CompileTestsApiFramework "CompileTestsApiFramework" ON)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT DEFINED DISABLE_MANUAL_FIND_PACKAGES)
    find_package(GTest REQUIRED)
    include(GoogleTest)
    find_package(Git)
    find_package(OpenSSL REQUIRED)
    find_package(PkgConfig)
endif()

if(NOT DEFINED Poco_LIBRARIES)
    message("Finding poco")
    find_package(Poco REQUIRED COMPONENTS Crypto JSON Net NetSSL Redis)
endif()

if(NOT DEFINED PISTACHE_LIBRARY)
    message("Finding pistache")
    pkg_check_modules(Pistache REQUIRED IMPORTED_TARGET libpistache)
    set(PISTACHE_LIBRARY PkgConfig::Pistache)
endif()

if (NOT DEFINED CURRENT_SHORT_MAIN_GIT_COMMIT)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULTS_VARIABLE GIT_REV_PARSE_SHORT_HEAD_RES
        OUTPUT_VARIABLE CURRENT_SHORT_MAIN_GIT_COMMIT_TMP
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message("Commit short ${CURRENT_SHORT_MAIN_GIT_COMMIT}")
    set(CURRENT_SHORT_MAIN_GIT_COMMIT ${CURRENT_SHORT_MAIN_GIT_COMMIT_TMP}
        CACHE STRING "CURRENT_SHORT_MAIN_GIT_COMMIT")
endif()

if (NOT DEFINED CURRENT_FULL_MAIN_GIT_COMMIT)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULTS_VARIABLE GIT_REV_PARSE_FULL_HEAD_RES
        OUTPUT_VARIABLE CURRENT_FULL_MAIN_GIT_COMMIT_TMP
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message("Commit full ${CURRENT_FULL_MAIN_GIT_COMMIT}")
    set(CURRENT_FULL_MAIN_GIT_COMMIT ${CURRENT_FULL_MAIN_GIT_COMMIT_TMP}
        CACHE STRING "CURRENT_FULL_MAIN_GIT_COMMIT")
endif()

if (NOT DEFINED CURRENT_JOB_VERSION)
    set(CURRENT_JOB_VERSION "@${CURRENT_SHORT_MAIN_GIT_COMMIT}" CACHE STRING "CURRENT_JOB_VERSION")
endif()

# setando todos arquivos fontes que serão compilados
file(
    GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CacheUtils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/WebInterface/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/models/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Authorization/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Database/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/queues/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jobhandler/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/WebControllers/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/WebSocket/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Storage/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/JSON/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/PistacheCustomHttpHeaders/*.cpp"
)

add_library(${projectname} STATIC ${SOURCES})

add_compile_definitions("CUR_JOB_FRAMEWORK_VERSION=\"${CURRENT_JOB_VERSION}\"")
add_compile_definitions("FULL_COMMIT_HASH=\"${CURRENT_FULL_MAIN_GIT_COMMIT}\"")
target_compile_definitions(${projectname} PRIVATE "PROJECT_NAME=\"cppapiframework\"")

# Compiler-specific compile flags
if (NOT DEFINED USING_COMPILER_FLAGS)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        message(STATUS "Setting G++ flags")
        # G++
        target_compile_options(${projectname} PRIVATE -Wall -Werror -Wextra -std=gnu++20 -Wformat-security -Wconversion -Wsign-conversion  -Wno-gnu -Wno-gnu-statement-expression)
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        message(STATUS "Setting MSVC flags")
        # MSVC
        target_compile_options(${projectname} PRIVATE /EHsc /W2 /c)
    elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message(STATUS "Setting Clang flags")
        set(USING_COMPILER_FLAGS -Weverything -Werror -Wno-unused-macros -std=gnu++20 -Wno-disabled-macro-expansion -Wpedantic -Wno-padded -Wno-constant-conversion -Wno-c++98-compat -Wno-padded -Wno-date-time -Wno-c++98-compat-pedantic -Wno-exit-time-destructors -Wno-global-constructors -Wno-gnu -Wno-gnu-statement-expression)
        target_compile_options(${PROJECT_NAME} PRIVATE ${USING_COMPILER_FLAGS})
        # Clang-tidy
        if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
            message(STATUS "Setting clang-tidy flags")
            set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-header-filter=${CMAKE_CURRENT_SOURCE_DIR}/src;--warnings-as-errors=\"*\";-extra-arg=-std=gnu++20;-extra-arg=-Wno-gnu;-export-fixes=clang-tidy-sugested-fixes.txt")
        endif()
    endif()
endif()

target_precompile_headers(${projectname} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/stdafx.hpp")

# verifica se o poco foi encontrado e faz o link com a lib
if(Poco_FOUND)
    message("-- Lib Poco encontrada!")
    target_link_libraries(${projectname} ${Poco_LIBRARIES})
else()
    message(FATAL_ERROR "-- Biblioteca poco não encontrada")
endif(Poco_FOUND)

if(DEFINED PISTACHE_LIBRARY OR Pistache_FOUND)
    message("-- Pistache encontrado!")
    target_link_libraries(${projectname} ${PISTACHE_LIBRARY})
else()
    message(FATAL_ERROR "-- Pistache is not found in your system.")
endif()

if(NOT DEFINED MYSQLCPPCONN_LIBRARY)
    set(MYSQLCPPCONN_LIBRARY mysqlcppconn)
endif()

target_link_libraries(${projectname} ${MYSQLCPPCONN_LIBRARY})

if ("${CompileTestsApiFramework}" STREQUAL "ON")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

