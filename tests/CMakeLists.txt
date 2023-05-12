cmake_minimum_required(VERSION 3.20)
project(tests)

set(CMAKE_CXX_STANDARD 20)

set(WCS_TESTS_DIR ${WCS_DIR}/tests)

file(GLOB_RECURSE CPPS "*.cpp")

add_executable(tests ${CPPS})

set(CATCH2_INCLUDE ${WCS_TESTS_DIR}/libs/catch2/include)

target_include_directories(
    tests PRIVATE
    ${CATCH2_INCLUDE}
)

target_link_libraries(
    tests PUBLIC
    ${WCS_LIB}
)

add_test(tests tests)

add_compile_definitions(WCS_EXCEPTION_WITHOUT_PLACE)
