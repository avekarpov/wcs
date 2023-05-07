#include <catch2/catch.hpp>

#include <wcs/backtest_engine.hpp>
#include <wcs/event_manager/event_manager.hpp>
#include <wcs/event_manager/to_virtual_exchange.h>
#include <wcs/event_manager/to_backtest_engine.h>
#include <wcs/input_file_stream.hpp>
#include <wcs/parsers/trade_parser.hpp>
#include <wcs/parsers/order_book_update_parser.hpp>
#include <wcs/data_stream.hpp>

#include "../fakes/order_controller.hpp"
#include "../fakes/order_book.hpp"
#include "../fakes/matching_engine.hpp"
#include "../fakes/virtual_exchange.hpp"

using namespace wcs;

using fakes::MatchingEngine;
using fakes::OrderBook;
using fakes::OrderController;
using fakes::VirtualExchange;

// TODO: change to fake
template <class EventManager_t>
using BacktestEngine = BacktestEngineBase<OrderController, OrderBook, MatchingEngine, EventManager_t>;

using TradeStream = InputFileStreamBase<TradeParser, true>;
using OrderBookUpdateStream = InputFileStreamBase<OrderBookUpdateParser, true>;

template <class EventManager_t>
using DataStream = DataStreamBase<TradeStream, OrderBookUpdateStream, EventManager_t>;

template <class EventManager_t>
using ToVirtualExchange = ToVirtualExchangeBase<VirtualExchange, EventManager_t>;

template <class EventManager_t>
using ToBacktestEngine = ToBacktestEngineBase<BacktestEngine, EventManager_t>;

using EventManager = EventManagerBase<ToVirtualExchange, ToBacktestEngine>;

TEST_CASE("EventManager")
{
    SECTION("Build")
    {
        // TODO: move stream test part in separate file

        auto event_manager = std::make_shared<EventManager>();
        auto backtest_engine = std::make_shared<BacktestEngine<EventManager>>();
        auto virtual_exchange = std::make_shared<VirtualExchange<EventManager>>();
        auto data_stream = std::make_shared<DataStream<EventManager>>();

        event_manager->setBacktestEngine(backtest_engine);
        event_manager->setVirtualExchange(virtual_exchange);
        backtest_engine->setEventManager(event_manager);
        data_stream->setEventManager(event_manager);

        backtest_engine->init();

        CHECK_THROWS(data_stream->tradeStream().setFilesQueue({}));
        CHECK_THROWS(data_stream->orderBookUpdateStream().setFilesQueue({}));

        {
            auto to_backtest_engine = std::static_pointer_cast<EventManager::ToBacktestEngine>(event_manager);
            to_backtest_engine->process(events::Trade { });
            to_backtest_engine->process(events::OrderBookUpdate { });
            to_backtest_engine->process(events::PlaceOrder<Side::Buy, OrderType::Market> { });
            to_backtest_engine->process(events::CancelOrder { });
        }

        {
            CHECK_THROWS(data_stream->processNextEvent());
        }
    }

    // TODO: add usage test
}
