#include <catch2/catch.hpp>

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

#include "../fakes/strategy.hpp"

using wcs::VirtualExchange;

template <class Consumer_t>
using MatchingEngine = wcs::MatchingEngine<Consumer_t, true>;

template <class EventManager_t>
using BacktestEngine =
    wcs::BacktestEngineBase<wcs::OrderController, wcs::OrderBook, MatchingEngine, EventManager_t>;

using TradeStream = wcs::InputFileStreamBase<wcs::TradeParser, true>;
using OrderBookUpdateStream = wcs::InputFileStreamBase<wcs::OrderBookUpdateParser, true>;

using EventManager =
    wcs::EventManagerBase<
        VirtualExchange,
        wcs::ToVirtualExchange,
        BacktestEngine,
        wcs::ToBacktestEngine,
        TradeStream,
        OrderBookUpdateStream>;

using Strategy = wcs::fakes::Strategy;

TEST_CASE("App")
{
    SECTION("Build")
    {
        auto event_manager = std::make_shared<EventManager>();
        auto virtual_exchange = std::make_shared<VirtualExchange<EventManager>>();
        auto backtest_engine = std::make_shared<BacktestEngine<EventManager>>();

        auto strategy = std::make_shared<Strategy>();

        event_manager->setVirtualExchange(virtual_exchange);
        event_manager->setBacktestEngine(backtest_engine);
        virtual_exchange->setEventManager(event_manager);
        backtest_engine->setEventManager(event_manager);

        virtual_exchange->setStrategy(strategy);
        strategy->setExchange(virtual_exchange);

        CHECK_THROWS(event_manager->tradeStream().setFilesQueue({ }));
        CHECK_THROWS(event_manager->orderBookUpdateStream().setFilesQueue({ }));

        CHECK_THROWS(event_manager->init());
        backtest_engine->init();

        while (!event_manager->empty()) {
            event_manager->processNextEvent();
        }
    }
}