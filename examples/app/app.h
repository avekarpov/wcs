#ifndef WCS_APP_H
#define WCS_APP_H

#include <wcs/event_manager/event_manager.hpp>
#include <wcs/event_manager/to_virtual_exchange.h>
#include <wcs/event_manager/to_backtest_engine.h>
#include <wcs/input_file_stream.hpp>
#include <wcs/parsers/trade_parser.hpp>
#include <wcs/parsers/order_book_update_parser.hpp>
#include <wcs/backtest_engine.hpp>
#include <wcs/order_controller.hpp>
#include <wcs/order_book.hpp>
#include <wcs/matching_engine.hpp>
#include <wcs/virtual_exchange.hpp>

using wcs::VirtualExchange;

using wcs::OrderController;
using wcs::OrderBook;
template <class Consumer_t>
using MatchingEngine = wcs::MatchingEngine<Consumer_t, false>;

template <class EventManager_t>
using BacktestEngine = wcs::BacktestEngineBase<OrderController, OrderBook, MatchingEngine, EventManager_t>;

using wcs::ToVirtualExchange;
using wcs::ToBacktestEngine;

using TradeStream = wcs::InputFileStreamBase<wcs::TradeParser, true>;
using OrderBookUpdateStream = wcs::InputFileStreamBase<wcs::OrderBookUpdateParser, true>;

using Eventmanager =
    wcs::EventManagerBase<
        VirtualExchange, ToVirtualExchange, BacktestEngine, ToBacktestEngine, TradeStream, OrderBookUpdateStream>;

using wcs::Strategy;

struct AppConfig
{
    wcs::Logger::Level _log_level;

    std::queue<std::filesystem::path> trad_files;
    std::queue<std::filesystem::path> order_book_update_files;

    wcs::Time delay;
};

class App
{
public:
    App(const AppConfig &config)
    :
        _event_manager { std::make_shared<Eventmanager>() },
        _virtual_exchange { std::make_shared<VirtualExchange<Eventmanager>>() },
        _backtest_engine { std::make_shared<BacktestEngine<Eventmanager>>() }
    {
        wcs::Logger::setLevel(config._log_level);

        _event_manager->tradeStream().setFilesQueue(config.trad_files);
        _event_manager->orderBookUpdateStream().setFilesQueue(config.order_book_update_files);

        _event_manager->setDelay(config.delay);
    }

    template <class Strategy_t>
    void setStrategy(const std::shared_ptr<Strategy_t> &strategy)
    {
        _strategy = strategy;
    }

    void init()
    {
        _event_manager->setVirtualExchange(_virtual_exchange);
        _event_manager->setBacktestEngine(_backtest_engine);
        _virtual_exchange->setEventManager(_event_manager);
        _backtest_engine->setEventManager(_event_manager);

        _virtual_exchange->setStrategy(_strategy);
        _strategy->setExchange(_virtual_exchange);

        _event_manager->init();
        _backtest_engine->init();
    }

    void run()
    {
        while (!_event_manager->empty()) {
            _event_manager->processNextEvent();
        }
    }

private:
    std::shared_ptr<Eventmanager> _event_manager;

    std::shared_ptr<VirtualExchange<Eventmanager>> _virtual_exchange;
    std::shared_ptr<BacktestEngine<Eventmanager>> _backtest_engine;

    std::shared_ptr<Strategy> _strategy;
};

#endif //WCS_APP_H
