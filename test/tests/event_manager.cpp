#include <catch2/catch.hpp>

#include <wcs/event_manager.hpp>
#include <wcs/backtest_engine.hpp>

#include "../fakes/order_controller.hpp"
#include "../fakes/order_book.hpp"
#include "../fakes/matching_engine.hpp"
#include "../fakes/virtual_exchange.hpp"

using namespace wcs;

using fakes::MatchingEngine;
using fakes::OrderBook;
using fakes::OrderController;
using fakes::VirtualExchange;

template <class EventManager_t>
// TODO: change to fake
using BacktestEngine = wcs::BacktestEngineBase<OrderController, OrderBook, MatchingEngine, EventManager_t>;

using EventManager = wcs::EventManagerBase<ToVirtualExchange, VirtualExchange, ToBacktestEngine, BacktestEngine>;

TEST_CASE("EventManager")
{
    SECTION("Build")
    {
        auto event_manager = std::make_shared<EventManager>();
        auto backtest_engine = std::make_shared<BacktestEngine<EventManager>>();
        
        event_manager->setBacktestEngine(backtest_engine);
        backtest_engine->setEventManager(event_manager);
    }
}
