#include <catch2/catch.hpp>

#include <wcs/orderbook.hpp>
#include <wcs/order_manager.hpp>

#include "../mocks/consumer.hpp"

using namespace wcs;

using mocks::Consumer;

TEST_CASE("Orderbook")
{
    auto consumer = std::make_shared<Consumer>();
    auto order_manager = std::make_shared<OrderManager>();
    
    Orderbook<Consumer> orderbook;
    orderbook.setConsumer(consumer);
    orderbook.setOrderManager(order_manager);
    
    order_manager->add<Side::Buy, OrderType::Limit>(OrderId { 1 }, Amount { 1 }, Price { 100 });
    order_manager->add<Side::Buy, OrderType::Limit>(OrderId { 2 }, Amount { 1 }, Price { 80 });
    order_manager->add<Side::Buy, OrderType::Limit>(OrderId { 3 }, Amount { 1 }, Price { 60 });
    order_manager->add<Side::Buy, OrderType::Limit>(OrderId { 4 }, Amount { 1 }, Price { 50 }); // out of orderbook
    
    order_manager->add<Side::Sell, OrderType::Limit>(OrderId { 5 }, Amount { 1 }, Price { 100 });
    order_manager->add<Side::Sell, OrderType::Limit>(OrderId { 6 }, Amount { 1 }, Price { 120 });
    order_manager->add<Side::Sell, OrderType::Limit>(OrderId { 7 }, Amount { 1 }, Price { 140 });
    order_manager->add<Side::Sell, OrderType::Limit>(OrderId { 8 }, Amount { 1 }, Price { 150 }); // out of orderbook
    
    SidePair<Depth> depth;
    events::OrderbookUpdate update { Ts { 0 }, EventId { 0 }, depth };
    
    SECTION("OrderbookUpdate")
    {
        update.ts = Ts { 1 };
        update.id = EventId { 1 };
    
        auto &buy = depth.get<Side::Buy>();
        buy.push_back(Level<Side::Buy> { Price { 100 }, Amount { 10 } });
        buy.push_back(Level<Side::Buy> { Price { 90 }, Amount { 10 } });
        buy.push_back(Level<Side::Buy> { Price { 70 }, Amount { 10 } });
        buy.push_back(Level<Side::Buy> { Price { 60 }, Amount { 10 } });
    
        auto &sell = depth.get<Side::Sell>();
        sell.push_back(Level<Side::Sell> { Price { 100 }, Amount { 10 }});
        sell.push_back(Level<Side::Sell> { Price { 110 }, Amount { 10 }});
        sell.push_back(Level<Side::Sell> { Price { 130 }, Amount { 10 }});
        sell.push_back(Level<Side::Sell> { Price { 140 }, Amount { 10 }});
        
        orderbook.processAndComplete(update);
    
        CHECK(buy[0] == Level<Side::Buy> { Price { 100 }, Amount { 11 } });
        CHECK(buy[1] == Level<Side::Buy> { Price { 90 }, Amount { 10 } });
        CHECK(buy[2] == Level<Side::Buy> { Price { 80 }, Amount { 1 } });
        CHECK(buy[3] == Level<Side::Buy> { Price { 70 }, Amount { 10 } });
        CHECK(buy[4] == Level<Side::Buy> { Price { 60 }, Amount { 11 } });
        CHECK(buy.size() == 5);
    
        CHECK(sell[0] == Level<Side::Sell> { Price { 100 }, Amount { 11 } });
        CHECK(sell[1] == Level<Side::Sell> { Price { 110 }, Amount { 10 } });
        CHECK(sell[2] == Level<Side::Sell> { Price { 120 }, Amount { 1 } });
        CHECK(sell[3] == Level<Side::Sell> { Price { 130 }, Amount { 10 } });
        CHECK(sell[4] == Level<Side::Sell> { Price { 140 }, Amount { 11 } });
        CHECK(sell.size() == 5);
    }
}
