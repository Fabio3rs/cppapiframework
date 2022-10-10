# Configuração dos testes no CMake/GoogleTest

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

set(srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
set(msrcdir "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(mainbinary "${CMAKE_CURRENT_SOURCE_DIR}/../bin")

file(
    GLOB ALL_TESTS_SOURCES 
    "${srcdir}/*.cpp"
)

add_executable(apiframework_test ${ALL_TESTS_SOURCES})

target_precompile_headers(apiframework_test PRIVATE "${msrcdir}/stdafx.hpp")

target_compile_options(apiframework_test PUBLIC ${USING_COMPILER_FLAGS})
target_link_libraries(apiframework_test GTest::GTest GTest::Main cppapiframework)

if(Poco_FOUND)
    message("-- Poco encontrado!")
    target_link_libraries(apiframework_test ${Poco_LIBRARIES})
else()
    message(FATAL_ERROR "-- Poco is not found in your system.")
endif(Poco_FOUND)

if(DEFINED PISTACHE_LIBRARY OR Pistache_FOUND)
    message("-- Pistache encontrado!")
    target_link_libraries(apiframework_test ${PISTACHE_LIBRARY})
else()
    message(FATAL_ERROR "-- Pistache is not found in your system.")
endif()

gtest_discover_tests(apiframework_test PROPERTIES
ENVIRONMENT "LLVM_PROFILE_FILE=apiframework_test-%p.profraw")


