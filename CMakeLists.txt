cmake_minimum_required(VERSION 3.20)
project(wcs)

set(CMAKE_CXX_STANDARD 20)

set(WCS_BUILD_TESTS OFF CACHE BOOL "Build library test")

# Add library
add_subdirectory(wcs)
set(WCS_INCLUDE wcs/include)
set(WCS_LIB wcs)

# Build library tests
if(WCS_BUILD_TESTS)
    add_subdirectory(test)
endif()
