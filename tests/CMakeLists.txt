# Configuração dos testes no CMake/GoogleTest

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
set(CMAKE_CXX_STANDARD 17)

set(srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
set(msrcdir "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(mainbinary "${CMAKE_CURRENT_SOURCE_DIR}/../bin")

file(
    GLOB UTILS_SOURCES 
    "${msrcdir}/utils/*.cpp"
)

file(
    GLOB JOBHANDLER_SOURCES 
    "${msrcdir}/jobhandler/*.cpp"
)

file(
    GLOB QUEUES_SOURCES 
    "${msrcdir}/queues/*.cpp"
)

file(
    GLOB ALL_TESTS_SOURCES 
    "${srcdir}/*.cpp"
)

set(TEST_COMPILE_FLAGS -Weverything -Werror -Wno-disabled-macro-expansion -Wno-global-constructors -std=c++17 -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-exit-time-destructors -Wno-global-constructors)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Werror -std=c++17 -Wno-c++98-compat -Wno-padded -Wno-date-time -Wno-c++98-compat-pedantic -Wno-exit-time-destructors -Wno-global-constructors")

add_executable(apiframework_test ${ALL_TESTS_SOURCES} ${QUEUES_SOURCES} ${UTILS_SOURCES} ${JOBHANDLER_SOURCES})

target_precompile_headers(apiframework_test PRIVATE "${msrcdir}/stdafx.hpp")

target_compile_options(apiframework_test PUBLIC ${TEST_COMPILE_FLAGS})
target_link_libraries(apiframework_test GTest::GTest GTest::Main)

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

