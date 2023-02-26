file(GLOB_RECURSE WCS_SRC "src/*.cpp")
add_library(wcs STATIC ${WCS_SRC})

add_subdirectory(libs/spdlog)
set(SPDLOG_INCLUDE libs/spdlog/include)

target_include_directories(
    wcs PUBLIC
    ${SPDLOG_INCLUDE}
    include
)

target_link_libraries(
    wcs PUBLIC
)
