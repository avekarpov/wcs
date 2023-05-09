cmake_minimum_required(VERSION 3.20)
project(test)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE CPPS "*.cpp")

add_executable(test ${CPPS})

set(CATCH2_INCLUDE libs/catch2/include)

target_include_directories(
    test PRIVATE
    ${CATCH2_INCLUDE}

    test PUBLIC
    ${WCS_INCLUDE}
)

target_link_libraries(
    test PUBLIC
    ${WCS_LIB}
)

add_test(test test)

add_compile_definitions(WCS_EXCEPTION_WITHOUT_PLACE)
