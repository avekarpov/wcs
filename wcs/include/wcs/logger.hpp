#ifndef WCS_LOGGER_HPP
#define WCS_LOGGER_HPP

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace wcs
{

class Logger
{
public:
    enum class Level
    {
        Critical,
        Error,
        Warning,
        Info,
        Debug,
        Trace
    };
    
    static void setLevel(Level level);
    
    explicit Logger(const std::string &name);
    
    template <class ... Args>
    const Logger &critical(Args &&...args) const
    {
        _logger->critical(std::forward<Args>(args)...);
        
        return *this;
    }
    
    template <class ... Args>
    const Logger &error(Args &&...args) const
    {
        _logger->error(std::forward<Args>(args)...);
        
        return *this;
    }
    
    template <class ... Args>
    const Logger &warning(Args &&...args) const
    {
        _logger->warn(std::forward<Args>(args)...);
        
        return *this;
    }
    
    template <class ... Args>
    const Logger &info(Args &&...args) const
    {
        _logger->info(std::forward<Args>(args)...);
        
        return *this;
    }
    
    template <class ... Args>
    Logger &debug(Args &&...args)
    {
        _logger->debug(std::forward<Args>(args)...);
        
        return *this;
    }
    
    template <class ... Args>
    const Logger &trace(Args &&...args) const
    {
        _logger->trace(std::forward<Args>(args)...);
        
        return *this;
    }
    
    template <class Event>
    const Logger &gotEvent(const Event &event)
    {
        trace(R"(Got event "{}" with [{}])", Event::Name, event);
        
        return *this;
    }

private:
    inline static constexpr auto PATTERN = "[%L] [%H:%M:%S %D] [%n]: %v";
    
    std::shared_ptr<spdlog::logger> _logger;
    
};

} // namespace wcs


#endif //WCS_LOGGER_HPP
