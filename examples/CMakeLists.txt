set(WCS_EXAMPLES_DIR ${WCS_DIR}/examples)

# TODO: make beauty
set(ARGUMENTUM_BUILD_STATIC_LIBS ON)
add_subdirectory(libs/argumentum/src)
set(ARGUMENTUM_INCLUDE ${WCS_EXAMPLES_DIR}/libs/argumentum/include)
set(ARGUMENTUM_LIB argumentum)

add_subdirectory(libs/simdjson)
set(SIMDJSON_INCLUDE ${WCS_EXAMPLES_DIR}/libs/simdjson/include)
SET(SIMDJSON_LIB simdjson)

add_subdirectory(app)
