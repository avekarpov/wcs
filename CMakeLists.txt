cmake_minimum_required(VERSION 3.20)
project(wcs)

set(CMAKE_CXX_STANDARD 20)

set(WCS_BUILD_TESTS OFF CACHE BOOL "Build library tests")
set(WCS_BUILD_EXAMPLES OFF CACHE BOOL "Build library examples")
set(WCS_ADDRESS_SANITIZER OFF CACHE BOOL "Build with address sanitizer")

# Build with address sanitizer
if(WCS_ADDRESS_SANITIZER)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libasan")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libasan")
endif()

# Add library
add_subdirectory(wcs)
set(WCS_INCLUDE wcs/include)
set(WCS_LIB wcs)

# Build library tests
if(WCS_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Build library examples
if(WCS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
