#include "../include/wcs/utilits/exception.hpp"
#include "../include/wcs/logger.hpp"

namespace wcs
{

void Logger::setLevel(wcs::Logger::Level level)
{
    switch (level)
    {
        case Level::Critical:
            spdlog::set_level(spdlog::level::critical);
            break;
        case Level::Warning:
            spdlog::set_level(spdlog::level::warn);
            break;
        case Level::Error:
            spdlog::set_level(spdlog::level::err);
            break;
        case Level::Info:
            spdlog::set_level(spdlog::level::info);
            break;
        case Level::Debug:
            spdlog::set_level(spdlog::level::debug);
            break;
        case Level::Trace:
            spdlog::set_level(spdlog::level::trace);
            break;
        default:
            throw WCS_EXCEPTION(std::invalid_argument, "Invalid level");
    }
}

Logger::Logger(const std::string &name) : _logger { spdlog::stderr_color_mt(name) }
{
    _logger->set_pattern(PATTERN);
}

} // namespace wcs
