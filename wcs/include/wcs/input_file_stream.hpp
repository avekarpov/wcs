#ifndef WCS_INPUT_FILE_STREAM_HPP
#define WCS_INPUT_FILE_STREAM_HPP

#include <filesystem>
#include <fstream>
#include <queue>

#include "logger.hpp"
#include "utilits/exception.hpp"

namespace wcs
{

class InputFileStreamLogger
{
protected:
    inline static Logger _logger { "InputFileStream" };
    
};

// TODO: add test
template <class Parser_t, bool SkipHeader>
class InputFileStreamBase : public InputFileStreamLogger
{
public:
    using Event = typename Parser_t::Event;

public:
    /// @warning input files must not have empty lines
    void setFilesQueue(const std::queue<std::filesystem::path> &files_queue)
    {
        _files_queue = files_queue;

        if (_files_queue.empty()) {
            throw WCS_EXCEPTION(std::invalid_argument, "Empty file queue");
        }

        openNextFile();
    }

    const Event &nextEvent()
    {
        return Parser_t::parse(nextEventString());
    }

    bool empty() const
    {
        return _files_queue.empty() && (!_current_file.is_open() || _current_file.eof());
    }
    
private:
    std::string &nextEventString()
    {
        if (_current_file.eof()) {
            openNextFile();
        }

        std::getline(_current_file, _buffer);

        assert(!_buffer.empty());

        _logger.trace(R"(Read next event string "{}")", _buffer);
    
        return _buffer;
    }
    
    void openNextFile()
    {
        if (_files_queue.empty()) {
            throw WCS_EXCEPTION(std::runtime_error, "End of file queue");
        }
        
        const auto &next_file = _files_queue.front();
        
        _logger.info(R"(Open next file "{}")", next_file.string());
        
        if (_current_file.is_open()) {
            _current_file.close();
        }
        _current_file.open(next_file);
        
        if constexpr (SkipHeader) {
            std::getline(_current_file, _buffer);
        }
        
        _files_queue.pop();
    }
    
private:
    std::queue<std::filesystem::path> _files_queue;
    
    std::ifstream _current_file;
    std::string _buffer;
};

}

#endif //WCS_INPUT_FILE_STREAM_HPP
