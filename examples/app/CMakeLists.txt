cmake_minimum_required(VERSION 3.20)
project(app)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE CPPS "*.cpp")

add_executable(app ${CPPS})

target_include_directories(
    app PRIVATE
    ${ARGUMENTUM_INCLUDE}
    ${SIMDJSON_INCLUDE}
)

target_link_libraries(
    app PUBLIC
    ${ARGUMENTUM_LIB}
    ${SIMDJSON_LIB}
    ${WCS_LIB}
)