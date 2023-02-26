file(GLOB_RECURSE WCS_SRC "src/*.cpp")
add_library(wcs STATIC ${WCS_SRC})

target_include_directories(
    wcs PUBLIC
)

target_link_libraries(
    wcs PUBLIC
)
