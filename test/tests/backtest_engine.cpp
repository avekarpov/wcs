#include <catch2/catch.hpp>

#include <wcs/backtest_engine.hpp>

#include "../fakes/event_manager.hpp"
#include "../fakes/order_controller.hpp"
#include "../fakes/order_book.hpp"
#include "../fakes/matching_engine.hpp"

using namespace wcs;

using EventManager = fakes::EventManager<fakes::ToVirtualExchange, fakes::ToBacktestEngine>;
using fakes::MatchingEngine;
using fakes::OrderBook;
using fakes::OrderController;

TEST_CASE("BacktestEngine")
{
    SECTION("Build")
    {
        auto backtest_engine =
            std::make_shared<BacktestEngineBase<OrderController, OrderBook, MatchingEngine, EventManager>>();
        
        backtest_engine->init();
        
        backtest_engine->process(events::PlaceOrder<Side::Buy, OrderType::Limit> { });
        backtest_engine->process(events::CancelOrder { });
        backtest_engine->process(events::DecreaseLevel<Side::Buy> { });
        backtest_engine->process(events::FillOrder { });
        backtest_engine->process(events::FreezeOrder { });
        backtest_engine->process(events::UnfreezeOrder { });
        backtest_engine->process(events::MoveOrderTo { });
        SidePair<Depth> depth;
        events::OrderBookUpdate update { .depth = depth };
        backtest_engine->processAndComplete(update);
        backtest_engine->process(events::OrderUpdate<OrderStatus::New> { });
        backtest_engine->process(events::OrderUpdate<OrderStatus::Placed> { });
        backtest_engine->process(events::ShiftOrder { });
        backtest_engine->process(events::Trade { });
    }
}
