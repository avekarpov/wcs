#include <catch2/catch.hpp>

#include <wcs/backtest_engine.hpp>
#include <wcs/event_manager/event_manager.hpp>
#include <wcs/event_manager/to_virtual_exchange.h>
#include <wcs/event_manager/to_backtest_engine.h>
#include <wcs/input_file_stream.hpp>
#include <wcs/parsers/trade_parser.hpp>
#include <wcs/parsers/order_book_update_parser.hpp>

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

// TODO: change to fake
using TradeStream = InputFileStreamBase<TradeParser, true>;
using OrderBookUpdateStream = InputFileStreamBase<OrderBookUpdateParser, true>;

using EventManager =
    EventManagerBase<
        VirtualExchange, ToVirtualExchange, BacktestEngine, ToBacktestEngine, TradeStream, OrderBookUpdateStream>;

TEST_CASE("EventManager")
{
    SECTION("Build")
    {
        auto event_manager = std::make_shared<EventManager>();
        auto backtest_engine = std::make_shared<BacktestEngine<EventManager>>();
        auto virtual_exchange = std::make_shared<VirtualExchange<EventManager>>();

        event_manager->setBacktestEngine(backtest_engine);
        event_manager->setVirtualExchange(virtual_exchange);
        backtest_engine->setEventManager(event_manager);

        event_manager->setDelay(Seconds { 5 });
        CHECK_THROWS(event_manager->tradeStream().setFilesQueue({ }));
        CHECK_THROWS(event_manager->orderBookUpdateStream().setFilesQueue({ }));

        CHECK_THROWS(event_manager->init());
        backtest_engine->init();

        {
            while (!event_manager->empty()) {
                event_manager->processNextEvent();
            }
        }
        {
            auto to_backtest_engine = std::static_pointer_cast<EventManager::ToBacktestEngine>(event_manager);
            to_backtest_engine->process(events::PlaceOrder<Side::Buy, OrderType::Market> { });
            to_backtest_engine->process(events::CancelOrder { });
        }
        {
            auto to_virtual_exchange = std::static_pointer_cast<EventManager::ToVirtualExchange>(event_manager);
            to_virtual_exchange->process(events::Trade { });
            to_virtual_exchange->process(events::OrderBookUpdate { });
            to_virtual_exchange->process(events::OrderUpdate<wcs::OrderStatus::Placed> { });
        }
    }

    // TODO: add usage test
}
