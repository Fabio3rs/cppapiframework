# Configuração dos testes no CMake/GoogleTest

include(CTest)
enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)

set(srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
set(msrcdir "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(mainbinary "${CMAKE_CURRENT_SOURCE_DIR}/../bin")

file(
    GLOB ALL_TESTS_SOURCES 
    "${srcdir}/*.cpp"
)

add_executable(apiframework_test ${ALL_TESTS_SOURCES})

target_precompile_headers(apiframework_test PRIVATE "${msrcdir}/projstdafx.hpp")

target_compile_options(apiframework_test PUBLIC ${USING_COMPILER_FLAGS})
target_link_libraries(apiframework_test GTest::GTest GTest::Main cppapiframework)

gtest_discover_tests(apiframework_test PROPERTIES
ENVIRONMENT "LLVM_PROFILE_FILE=apiframework_test-%p.profraw")


