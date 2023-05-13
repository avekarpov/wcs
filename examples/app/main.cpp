#include <argumentum/argparse.h>
#include <simdjson.h>

#include "app.h"
#include "simple_strategy.hpp"

auto parseDataQueue(std::string_view file_path)
{
    static wcs::Logger logger { "parseDataQueue" };

    using namespace simdjson;
    using DataQueue = std::queue<std::filesystem::path>;

    try {
        ondemand::parser parser;
        auto json = padded_string::load(file_path);
        auto doc = parser.iterate(json);

        DataQueue trades_queue;
        DataQueue order_book_updates_queue;

        auto trades = doc["trades"].get_array();
        for (std::string_view path : trades) {
            trades_queue.emplace(path);
        }

        auto order_book_updates = doc["order_book_updates"].get_array();
        for (std::string_view path : order_book_updates) {
            order_book_updates_queue.emplace(path);
        }

        return std::pair<DataQueue, DataQueue> { trades_queue, order_book_updates_queue};
    }
    catch (...) {
        logger.error(
            "Data queue parsing file error\n"
            "file format:\n"
            "{\n"
            "    \"trades\": [ \"path/to/data/1\", \"path/to/data/2\", ... ],\n"
            "    \"order_book_updates\": [ \"path/to/data/1\", \"path/to/data/2\", ... ]\n"
            "}\n"
        );

        throw;
    }
}

int main(int argc, char *argv[])
{
    auto parser = argumentum::argument_parser { };
    auto params = parser.params();

    parser.config().program(argv[0]).description( "Backtest example" );

    wcs::Logger::Level log_level;
    params.add_parameter(log_level, "-l")
        .maxargs(1)
        .absent(wcs::Logger::Level::Info)
        .action([] (auto &log_level, const std::string &value)
        {
            if (value == "info") {
                log_level = wcs::Logger::Level::Info;
            }
            else if (value == "debug") {
                log_level = wcs::Logger::Level::Debug;
            }
            else if (value == "trace") {
                log_level = wcs::Logger::Level::Trace;
            }
            else {
                throw std::invalid_argument {"Invalid log level"};
            }
        })
        .help("Log level");

    std::string data_queue_file_path;
    params.add_parameter(data_queue_file_path, "-f")
    .nargs(1)
    .help("Data queue file path");

    wcs::Seconds delay;
    params.add_parameter(delay, "-d")
    .nargs(1)
    .absent(wcs::Seconds { 0 })
    .action([] (auto &delay, const std::string &value)
    {
        delay = wcs::Seconds { std::stol(value) };
    })
    .help("Event transfer delay between backtest and strategy");

    if (!parser.parse_args(argc, argv)) {
        return 0;
    }

    auto [trades_files, order_book_updates_files] = parseDataQueue(data_queue_file_path);

    AppConfig config
    {
        log_level,
        trades_files,
        order_book_updates_files,
        delay
    };

    auto strategy = std::make_shared<SimpleStrategy>(wcs::Seconds { 10 }, 0.42);

    App app { config };
    app.setStrategy(strategy);

    app.init();

    app.run();

    return 0;
}