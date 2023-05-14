#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <wcs/logger.hpp>

using namespace wcs;

int main(int argc, char *argv[])
{
    Logger::setLevel(Logger::Level::Critical);
    
    return Catch::Session().run(argc, argv);
}
