# Configuração dos testes no CMake/GoogleTest

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
set(CMAKE_CXX_STANDARD 17)

set(srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
set(msrcdir "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(mainbinary "${CMAKE_CURRENT_SOURCE_DIR}/../bin")

file(
    GLOB ALL_TESTS_SOURCES 
    "${srcdir}/*.cpp"
)

add_executable(apiframework_test ${ALL_TESTS_SOURCES})

# Compiler-specific compile flags
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    message(STATUS "Setting G++ flags")
    # G++
    target_compile_options(apiframework_test PRIVATE -Wall -Werror -Wextra -std=c++17 -Wformat-security -Wconversion -Wsign-conversion)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
    message(STATUS "Setting MSVC flags")
    # MSVC
    target_compile_options(apiframework_test PRIVATE /EHsc /W2 /c)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    message(STATUS "Setting Clang flags")
    target_compile_options(apiframework_test PRIVATE -o2 -Weverything -Werror -std=c++17 -Wno-disabled-macro-expansion -Wpedantic -Wno-padded -Wno-constant-conversion -Wno-c++98-compat -Wno-padded -Wno-date-time -Wno-c++98-compat-pedantic -Wno-exit-time-destructors -Wno-global-constructors)
    # Clang-tidy
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-header-filter=${CMAKE_CURRENT_SOURCE_DIR}/src;--warnings-as-errors=\"*\";-extra-arg=-std=c++17;-export-fixes;-checks=read*,modern*,hicpp*,performance*")
endif()

target_precompile_headers(apiframework_test PRIVATE "${msrcdir}/stdafx.hpp")

target_compile_options(apiframework_test PUBLIC ${TEST_COMPILE_FLAGS})
target_link_libraries(apiframework_test GTest::GTest GTest::Main cppapiframework)

if(Poco_FOUND)
    message("-- Poco encontrado!")
    target_link_libraries(apiframework_test ${Poco_LIBRARIES})
else()
    message(FATAL_ERROR "-- Poco is not found in your system.")
endif(Poco_FOUND)

if(Pistache_FOUND)
    message("-- Pistache encontrado!")
    target_link_libraries(apiframework_test PkgConfig::Pistache)
else()
    message(FATAL_ERROR "-- Pistache is not found in your system.")
endif(Pistache_FOUND)

gtest_discover_tests(apiframework_test)

