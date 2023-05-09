#include <catch2/catch.hpp>

#include <wcs/matching_engine.hpp>
#include <wcs/order_controller.hpp>

#include "../spies/consumer.hpp"
#include "../utilits/side_section.hpp"

using namespace wcs;

using spies::Consumer;

template <Side S, Side Opposite = opposite(S)>
void marketOrders(
    std::shared_ptr<Consumer> consumer,
    std::shared_ptr<OrderController<Consumer>> order_controller,
    std::shared_ptr<MatchingEngine<Consumer, true>> matching_engine
) {
    const auto place_order = [&order_controller] (OrderId id, const Amount &amount)
    {
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Market>>(
            TimeManager::time(),
            id,
            amount
        ));
    };
    
    place_order(OrderId { 1 }, Amount { 1 });
    place_order(OrderId { 2 }, Amount { 5 });
    place_order(OrderId { 3 }, Amount { 9 });
    
    consumer->clear();
    
    matching_engine->process(EventBuilder::build<events::Trade>(
        TimeManager::time(),
        TradeId { 1 },
        Price { },
        Amount { 3 },
        Opposite
    ));
    
    matching_engine->process(EventBuilder::build<events::Trade>(
        TimeManager::time(),
        TradeId { 2 },
        Price { },
        Amount { 6 },
        Opposite
    ));
    
    matching_engine->process(EventBuilder::build<events::Trade>(
        TimeManager::time(),
        TradeId { 2 },
        Price { },
        Amount { 18 },
        Opposite
    ));
    
    const auto fill_order_events = consumer->fillOrderEvents();
    auto fill_order = fill_order_events.begin();
    CHECK(fill_order->client_order_id == OrderId { 1 });
    CHECK(fill_order->amount == Amount { 1 });
    ++fill_order;
    CHECK(fill_order->client_order_id == OrderId { 2 });
    CHECK(fill_order->amount == Amount { 2 });
    ++fill_order;
    CHECK(fill_order->client_order_id == OrderId { 2 });
    CHECK(fill_order->amount == Amount { 3 });
    ++fill_order;
    CHECK(fill_order->client_order_id == OrderId { 3 });
    CHECK(fill_order->amount == Amount { 3 });
    ++fill_order;
    CHECK(fill_order->client_order_id == OrderId { 3 });
    CHECK(fill_order->amount == Amount { 6 });
    CHECK(fill_order_events.size() == 5);
}

template <Side S>
void limitOrders(
    std::shared_ptr<Consumer> consumer,
    std::shared_ptr<OrderController<Consumer>> order_controller,
    std::shared_ptr<MatchingEngine<Consumer, true>> matching_engine
) {
    const auto place_order = [&order_controller] (OrderId id, const Price &price, const Amount &amount)
    {
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            TimeManager::time(),
            id,
            price,
            amount
        ));
    };
    
    {
        std::vector<std::tuple<OrderId, Price, Amount>> orders;
        
        if constexpr (S == Side::Buy) {
            orders = std::vector<std::tuple<OrderId, Price, Amount>> {
                { OrderId { 1 }, Price { 100 }, Amount { 100 }},
                { OrderId { 2 }, Price { 100 }, Amount { 50 }},
                { OrderId { 3 }, Price { 90 }, Amount { 7 }},
                { OrderId { 4 }, Price { 90 }, Amount { 1 }},
                { OrderId { 5 }, Price { 90 }, Amount { 5 }},
                { OrderId { 6 }, Price { 90 }, Amount { 9 }},
                { OrderId { 7 }, Price { 80 }, Amount { 50 }},
                { OrderId { 8 }, Price { 80 }, Amount { 100 }},
            };
        }
        else {
            orders = std::vector<std::tuple<OrderId, Price, Amount>> {
                { OrderId { 1 }, Price { 80 }, Amount { 100 }},
                { OrderId { 2 }, Price { 80 }, Amount { 50 }},
                { OrderId { 3 }, Price { 90 }, Amount { 7 }},
                { OrderId { 4 }, Price { 90 }, Amount { 1 }},
                { OrderId { 5 }, Price { 90 }, Amount { 5 }},
                { OrderId { 6 }, Price { 90 }, Amount { 9 }},
                { OrderId { 7 }, Price { 100 }, Amount { 50 }},
                { OrderId { 8 }, Price { 100 }, Amount { 100 }},
            };
        }
        
        for (const auto &order : orders) {
            place_order(get<0>(order), get<1>(order), get<2>(order));
            order_controller->process(EventBuilder::build<events::MoveOrderTo>(
                TimeManager::time(),
                get<0>(order),
                Amount { 10 }
            ));
        }
    }
    
    consumer->clear();
    
    matching_engine->process(EventBuilder::build<events::Trade>(
        TimeManager::time(),
        TradeId { 1 },
        Price { 90 },
        Amount { 15 },
        S
    ));
    
    matching_engine->process(EventBuilder::build<events::Trade>(
        TimeManager::time(),
        TradeId { 2 },
        Price { 90 },
        Amount { 3 },
        S
    ));
    
    matching_engine->process(EventBuilder::build<events::Trade>(
        TimeManager::time(),
        TradeId { 3 },
        Price { 90 },
        Amount { 6 },
        S
    ));
    
    matching_engine->process(EventBuilder::build<events::Trade>(
        TimeManager::time(),
        TradeId { 4 },
        Price { 90 },
        Amount { 20 },
        S
    ));
    
    const auto shift_order_events = consumer->shiftOrderEvents();
    // First, second orders on another level, third order must fill immediately
    for (OrderId id = 4; const auto &shift_order : shift_order_events) {
        CHECK(shift_order.client_order_id == id);
        CHECK(shift_order.volume == Amount { 10 });
        
        ++id;
    }
    CHECK(shift_order_events.size() == 3);
    
    const auto &fill_order_events = consumer->fillOrderEvents();
    auto fill_order = fill_order_events.begin();
    CHECK(fill_order->client_order_id == OrderId { 3 });
    CHECK(fill_order->amount == Amount { 5 });
    ++fill_order;
    CHECK(fill_order->client_order_id == OrderId { 3 });
    CHECK(fill_order->amount == Amount { 2 });
    ++fill_order;
    CHECK(fill_order->client_order_id == OrderId { 4 });
    CHECK(fill_order->amount == Amount { 1 });
    ++fill_order;
    CHECK(fill_order->client_order_id == OrderId { 5 });
    CHECK(fill_order->amount == Amount { 5 });
    ++fill_order;
    CHECK(fill_order->client_order_id == OrderId { 6 });
    CHECK(fill_order->amount == Amount { 1 });
    ++fill_order;
    CHECK(fill_order->client_order_id == OrderId { 6 });
    CHECK(fill_order->amount == Amount { 8 });
    CHECK(fill_order_events.size() == 6);
    
    const auto &decrease_level_events = consumer->decreaseLevelEvents();
    auto decrease_level = decrease_level_events.begin();
    CHECK(std::get<events::DecreaseLevel<S>>(*decrease_level).price == Price { 90 });
    CHECK(std::get<events::DecreaseLevel<S>>(*decrease_level).volume == Amount { 10 });
    ++decrease_level;
    CHECK(std::get<events::DecreaseLevel<S>>(*decrease_level).price == Price { 90 });
    CHECK(std::get<events::DecreaseLevel<S>>(*decrease_level).volume == Amount { 12 });
    CHECK(decrease_level_events.size() == 2);
}

TEST_CASE("MatchingEngine")
{
    auto consumer = std::make_shared<Consumer>();
    
    auto order_controller = std::make_shared<OrderController<Consumer>>();
    order_controller->setConsumer(consumer);
    consumer->setOrderController(order_controller);
    
    auto matching_engine = std::make_shared<MatchingEngine<Consumer, true>>();
    matching_engine->setConsumer(consumer);
    matching_engine->setOrderManager(order_controller->orderManager());
    consumer->setMatchingEngine(matching_engine);
    
    SIDE_SECTION("Fill market orders", marketOrders, consumer, order_controller, matching_engine);
    
    SIDE_SECTION("Fill limit orders and decrease level", limitOrders, consumer, order_controller, matching_engine);
}
