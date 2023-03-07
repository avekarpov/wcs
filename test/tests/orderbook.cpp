#include <catch2/catch.hpp>

#include <wcs/orderbook.hpp>
#include <wcs/order_manager.hpp>
#include <wcs/order_controller.hpp>

#include "../spies/consumer.hpp"
#include "../utilits/event_builder.hpp"

using namespace wcs;

using spies::Consumer;

TEST_CASE("Orderbook")
{
    auto consumer = std::make_shared<Consumer>();
    
    auto order_controller = std::make_shared<OrderController<Consumer>>();
    order_controller->setConsumer(consumer);
    consumer->setOrderController(order_controller);
    
    auto orderbook = std::make_shared<Orderbook<Consumer>>();
    orderbook->setConsumer(consumer);
    orderbook->setOrderManager(order_controller->orderManager());
    consumer->setOrderbook(orderbook);
    
    SidePair<Depth> depth;
    events::OrderbookUpdate update { Ts { 0 }, EventId { 0 }, depth };
    
    // Preparing, first event must be orderbook update
    {
        update = createEvent<events::OrderbookUpdate>(depth);
    
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
        
        orderbook->processAndComplete(update);
    
        CHECK(buy[0] == Level<Side::Buy> { Price { 100 }, Amount { 10 } });
        CHECK(buy[1] == Level<Side::Buy> { Price { 90 }, Amount { 10 } });
        CHECK(buy[2] == Level<Side::Buy> { Price { 70 }, Amount { 10 } });
        CHECK(buy[3] == Level<Side::Buy> { Price { 60 }, Amount { 10 } });
        CHECK(buy.size() == 4);
    
        CHECK(sell[0] == Level<Side::Sell> { Price { 100 }, Amount { 10 } });
        CHECK(sell[1] == Level<Side::Sell> { Price { 110 }, Amount { 10 } });
        CHECK(sell[2] == Level<Side::Sell> { Price { 130 }, Amount { 10 } });
        CHECK(sell[3] == Level<Side::Sell> { Price { 140 }, Amount { 10 } });
        CHECK(sell.size() == 4);
    }
    
    {
        auto place_buy_order = [&order_controller](OrderId client_order_id, const Amount &amount, const Price &price)
        {
            order_controller->process(createEvent<events::PlaceOrder<Side::Buy, OrderType::Limit>>
            (
                client_order_id,
                amount,
                price
            ));
        };
    
        auto place_sell_order = [&order_controller](OrderId client_order_id, const Amount &amount, const Price &price)
        {
            order_controller->process(createEvent<events::PlaceOrder<Side::Sell, OrderType::Limit>>
            (
                client_order_id,
                amount,
                price
            ));
        };
        
        place_buy_order(OrderId { 1 }, Amount { 1 }, Price { 100 });
        place_buy_order(OrderId { 2 }, Amount { 1 }, Price { 80 });
        place_buy_order(OrderId { 3 }, Amount { 1 }, Price { 60 });
        place_buy_order(OrderId { 4 }, Amount { 1 }, Price { 50 }); // out of orderbook
    
        place_sell_order(OrderId { 5 }, Amount { 1 }, Price { 100 });
        place_sell_order(OrderId { 6 }, Amount { 1 }, Price { 120 });
        place_sell_order(OrderId { 7 }, Amount { 1 }, Price { 140 });
        place_sell_order(OrderId { 8 }, Amount { 1 }, Price { 150 }); // out of orderbook
    }
    
    SECTION("Orderbook update")
    {
        consumer->clear();
        
        update.ts = Ts { 2 };
        update.id = EventId { 2 };
        
        auto &buy = depth.get<Side::Buy>(); buy.clear();
        buy.push_back(Level<Side::Buy> { Price { 100 }, Amount { 10 } });
        buy.push_back(Level<Side::Buy> { Price { 90 }, Amount { 10 } });
        buy.push_back(Level<Side::Buy> { Price { 70 }, Amount { 10 } });
        buy.push_back(Level<Side::Buy> { Price { 60 }, Amount { 10 } });
    
        auto &sell = depth.get<Side::Sell>(); sell.clear();
        sell.push_back(Level<Side::Sell> { Price { 100 }, Amount { 10 }});
        sell.push_back(Level<Side::Sell> { Price { 110 }, Amount { 10 }});
        sell.push_back(Level<Side::Sell> { Price { 130 }, Amount { 10 }});
        sell.push_back(Level<Side::Sell> { Price { 140 }, Amount { 10 }});
        
        orderbook->processAndComplete(update);
    
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
    
    SECTION("Moving order")
    {
        consumer->clear();
        
        update.ts = Ts { 2 };
        update.id = EventId { 2 };
    
        auto &buy = depth.get<Side::Buy>(); buy.clear();
        buy.push_back(Level<Side::Buy> { Price { 100 }, Amount { 5 } });
        buy.push_back(Level<Side::Buy> { Price { 95 }, Amount { 5 } });
        buy.push_back(Level<Side::Buy> { Price { 90 }, Amount { 5 } });
        buy.push_back(Level<Side::Buy> { Price { 70 }, Amount { 5 } });
    
        auto &sell = depth.get<Side::Sell>(); sell.clear();
        sell.push_back(Level<Side::Sell> { Price { 100 }, Amount { 5 }});
        sell.push_back(Level<Side::Sell> { Price { 105 }, Amount { 5 }});
        sell.push_back(Level<Side::Sell> { Price { 110 }, Amount { 5 }});
        sell.push_back(Level<Side::Sell> { Price { 130 }, Amount { 5 }});
    
        orderbook->processAndComplete(update);
        
        const auto &move_orders = consumer->moveOrders();
    
        CHECK(move_orders[0].client_order_id == OrderId { 1 });
        CHECK(move_orders[0].volume_before == Amount { 5 });
        CHECK(move_orders[1].client_order_id == OrderId { 5 });
        CHECK(move_orders[1].volume_before == Amount { 5 });
    }
}
