file(GLOB_RECURSE WCS_SRC "src/*.cpp")
add_library(wcs STATIC ${WCS_SRC})

add_subdirectory(libs/spdlog)
set(SPDLOG_INCLUDE libs/spdlog/include)

add_subdirectory(libs/fast_float)
set(FAST_FLOAT_INCLUDE libs/fast_float/include)
set(FAST_FLOAT_LIB fast_float)

target_include_directories(
    wcs PUBLIC
    ${SPDLOG_INCLUDE}
    ${FAST_FLOAT_INCLUDE}
    include
)

target_link_libraries(
    wcs PUBLIC
    ${FAST_FLOAT_LIB}
)
