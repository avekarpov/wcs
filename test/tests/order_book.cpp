#include <catch2/catch.hpp>

#include <wcs/order_book.hpp>
#include <wcs/order_controller.hpp>
#include <wcs/order_manager.hpp>

#include "../spies/consumer.hpp"
#include "../utilits/event_builder.hpp"

using namespace wcs;

using spies::Consumer;

TEST_CASE("OrderBook")
{
    auto consumer = std::make_shared<Consumer>();
    
    auto order_controller = std::make_shared<OrderController<Consumer>>();
    order_controller->setConsumer(consumer);
    consumer->setOrderController(order_controller);
    
    auto order_book = std::make_shared<OrderBook<Consumer>>();
    order_book->setConsumer(consumer);
    order_book->setOrderManager(order_controller->orderManager());
    consumer->setOrderBook(order_book);
    
    SidePair<Depth> depth;
    events::OrderBookUpdate update { .depth = depth };
    
    // Preparing, first update
    {
        // Creating event
        {
            auto &buy = depth.get<Side::Buy>();
            buy.push_back(Level<Side::Buy> { Price { 80 }, Amount { 8 }});
            // skip 70
            buy.push_back(Level<Side::Buy> { Price { 60 }, Amount { 6 }});
            buy.push_back(Level<Side::Buy> { Price { 50 }, Amount { 5 }});
            buy.push_back(Level<Side::Buy> { Price { 40 }, Amount { 4 }});
            // skip 30
            buy.push_back(Level<Side::Buy> { Price { 20 }, Amount { 2 }});
            // skip 10
            buy.push_back(Level<Side::Buy> { Price { 5 }, Amount { 0.5 }});
        
            auto &sell = depth.get<Side::Sell>();
            sell.push_back(Level<Side::Sell> { Price { 120 }, Amount { 12 }});
            //skip 130
            sell.push_back(Level<Side::Sell> { Price { 140 }, Amount { 14 }});
            sell.push_back(Level<Side::Sell> { Price { 150 }, Amount { 15 }});
            sell.push_back(Level<Side::Sell> { Price { 160 }, Amount { 16 }});
            // skip 170
            sell.push_back(Level<Side::Sell> { Price { 180 }, Amount { 18 }});
            //skip 190
            sell.push_back(Level<Side::Sell> { Price { 195 }, Amount { 19.5 }});
        }
    
        update = createEvent<events::OrderBookUpdate>(depth);
        order_book->processAndComplete(update);
        
        const auto &buy = order_book->historicalDepth().get<Side::Buy>();
        auto buy_it = buy.begin();
        REQUIRE(*buy_it == Level<Side::Buy> { Price { 80 }, Amount { 8 }});
        ++buy_it;
        REQUIRE(*buy_it == Level<Side::Buy> { Price { 60 }, Amount { 6 }});
        ++buy_it;
        REQUIRE(*buy_it == Level<Side::Buy> { Price { 50 }, Amount { 5 }});
        ++buy_it;
        REQUIRE(*buy_it == Level<Side::Buy> { Price { 40 }, Amount { 4 }});
        ++buy_it;
        REQUIRE(*buy_it == Level<Side::Buy> { Price { 20 }, Amount { 2 }});
        ++buy_it;
        REQUIRE(*buy_it == Level<Side::Buy> { Price { 5 }, Amount { 0.5 }});
        REQUIRE(buy.size() == 6);
    
        const auto &sell = order_book->historicalDepth().get<Side::Sell>();
        auto sell_it = sell.begin();
        REQUIRE(*sell_it == Level<Side::Sell> { Price { 120 }, Amount { 12 }});
        ++sell_it;
        REQUIRE(*sell_it == Level<Side::Sell> { Price { 140 }, Amount { 14 }});
        ++sell_it;
        REQUIRE(*sell_it == Level<Side::Sell> { Price { 150 }, Amount { 15 }});
        ++sell_it;
        REQUIRE(*sell_it == Level<Side::Sell> { Price { 160 }, Amount { 16 }});
        ++sell_it;
        REQUIRE(*sell_it == Level<Side::Sell> { Price { 180 }, Amount { 18 }});
        ++sell_it;
        REQUIRE(*sell_it == Level<Side::Sell> { Price { 195 }, Amount { 19.5 }});
        REQUIRE(sell.size() == 6);
    }
    
    auto place_buy_order = [&order_controller] (OrderId id, const Price &price, const Amount &amount)
    {
        order_controller->process(createEvent<events::PlaceOrder<Side::Buy, OrderType::Limit>>(
            id,
            price,
            amount
        ));
    };
    
    auto place_sell_order = [&order_controller] (OrderId id, const Price &price, const Amount &amount)
    {
        order_controller->process(createEvent<events::PlaceOrder<Side::Sell, OrderType::Limit>>(
            id,
            price,
            amount
        ));
    };
    
    SECTION("Placing order")
    {
        consumer->clear();
        
        SECTION("On empty price level")
        {
            SECTION("In order book")
            {
                SECTION("Spread")
                {
                    place_buy_order(OrderId { 1 }, Price { 90 }, Amount { 10 });
                    place_buy_order(OrderId { 2 }, Price { 90 }, Amount { 5 });
                    
                    place_sell_order(OrderId { 3 }, Price { 110 }, Amount { 10 });
                    place_sell_order(OrderId { 4 }, Price { 110 }, Amount { 5 });
                    
                    const auto &move_order_events = consumer->moveOrderEvents();
                    auto move = move_order_events.begin();
                    CHECK(move->client_order_id == OrderId { 1 });
                    CHECK(move->volume_before == Amount { 0 });
                    ++move;
                    CHECK(move->client_order_id == OrderId { 2 });
                    CHECK(move->volume_before == Amount { 0 });
                    ++move;
                    CHECK(move->client_order_id == OrderId { 3 });
                    CHECK(move->volume_before == Amount { 0 });
                    ++move;
                    CHECK(move->client_order_id == OrderId { 4 });
                    CHECK(move->volume_before == Amount { 0 });
                    CHECK(move_order_events.size() == 4);
                    
                    CHECK(consumer->freezeOrderEvents().empty());
                }
                
                SECTION("Not spread")
                {
                    place_buy_order(OrderId { 1 }, Price { 70 }, Amount { 10 });
                    place_buy_order(OrderId { 2 }, Price { 70 }, Amount { 5 });
                    
                    place_sell_order(OrderId { 3 }, Price { 130 }, Amount { 10 });
                    place_sell_order(OrderId { 4 }, Price { 130 }, Amount { 5 });
                    
                    const auto &move_order_events = consumer->moveOrderEvents();
                    auto move = move_order_events.begin();
                    CHECK(move->client_order_id == OrderId { 1 });
                    CHECK(move->volume_before == Amount { 0 });
                    ++move;
                    CHECK(move->client_order_id == OrderId { 2 });
                    CHECK(move->volume_before == Amount { 0 });
                    ++move;
                    CHECK(move->client_order_id == OrderId { 3 });
                    CHECK(move->volume_before == Amount { 0 });
                    ++move;
                    CHECK(move->client_order_id == OrderId { 4 });
                    CHECK(move->volume_before == Amount { 0 });
                    CHECK(move_order_events.size() == 4);
                    
                    CHECK(consumer->freezeOrderEvents().empty());
                }
            }
            
            SECTION("Out of order book")
            {
                place_buy_order(OrderId { 1 }, Price { 0 }, Amount { 10 });
                place_buy_order(OrderId { 2 }, Price { 0 }, Amount { 5 });
                
                place_sell_order(OrderId { 3 }, Price { 200 }, Amount { 10 });
                place_sell_order(OrderId { 4 }, Price { 200 }, Amount { 5 });
                
                const auto &freeze_order_events = consumer->freezeOrderEvents();
                auto freeze = freeze_order_events.begin();
                CHECK(freeze->client_order_id == OrderId { 1 });
                ++freeze;
                CHECK(freeze->client_order_id == OrderId { 2 });
                ++freeze;
                CHECK(freeze->client_order_id == OrderId { 3 });
                ++freeze;
                CHECK(freeze->client_order_id == OrderId { 4 });
                CHECK(freeze_order_events.size() == 4);
                
                CHECK(consumer->moveOrderEvents().empty());
            }
        }
    
        SECTION("On not empty price level")
        {
            place_buy_order(OrderId { 1 }, Price { 80 }, Amount { 10 });
            place_buy_order(OrderId { 2 }, Price { 80 }, Amount { 50 });
        
            place_sell_order(OrderId { 3 }, Price { 120 }, Amount { 10 });
            place_sell_order(OrderId { 4 }, Price { 120 }, Amount { 50 });
        
            const auto &move_order_events = consumer->moveOrderEvents();
            auto move = move_order_events.begin();
            CHECK(move->client_order_id == OrderId { 1 });
            CHECK(move->volume_before == Amount { 8 });
            ++move;
            CHECK(move->client_order_id == OrderId { 2 });
            CHECK(move->volume_before == Amount { 8 });
            ++move;
            CHECK(move->client_order_id == OrderId { 3 });
            CHECK(move->volume_before == Amount { 12 });
            ++move;
            CHECK(move->client_order_id == OrderId { 4 });
            CHECK(move->volume_before == Amount { 12 });
            CHECK(move_order_events.size() == 4);
        
            CHECK(consumer->freezeOrderEvents().empty());
        }
    }
    
    SECTION("OrderBook update")
    {
        SECTION("Depths")
        {
            auto update_order_book = [&order_book, &depth, &update] (const double &delta = 0)
            {
                // Creating event
                {
                    auto &buy = depth.get<Side::Buy>();
                    buy.clear();
                    buy.push_back(Level<Side::Buy> { Price { 98 + delta }, Amount { 1 }});
                    buy.push_back(Level<Side::Buy> { Price { 90 + delta }, Amount { 9 }});
                    buy.push_back(Level<Side::Buy> { Price { 80 + delta }, Amount { 40 }});
                    buy.push_back(Level<Side::Buy> { Price { 70 + delta }, Amount { 7 }});
                    buy.push_back(Level<Side::Buy> { Price { 60 + delta }, Amount { 30 }});
                    // skip 50
                    buy.push_back(Level<Side::Buy> { Price { 40 + delta}, Amount { 20 }});
                    buy.push_back(Level<Side::Buy> { Price { 30 + delta}, Amount { 3 }});
                    buy.push_back(Level<Side::Buy> { Price { 20 + delta}, Amount { 10 }});
            
                    auto &sell = depth.get<Side::Sell>();
                    sell.clear();
                    sell.push_back(Level<Side::Sell> { Price { 102 + delta}, Amount { 1 }});
                    sell.push_back(Level<Side::Sell> { Price { 110 + delta}, Amount { 11 }});
                    sell.push_back(Level<Side::Sell> { Price { 120 + delta}, Amount { 60 }});
                    sell.push_back(Level<Side::Sell> { Price { 130 + delta}, Amount { 13 }});
                    sell.push_back(Level<Side::Sell> { Price { 140 + delta}, Amount { 70 }});
                    // skip 150
                    sell.push_back(Level<Side::Sell> { Price { 160 + delta}, Amount { 80 }});
                    sell.push_back(Level<Side::Sell> { Price { 170 + delta}, Amount { 17 }});
                    sell.push_back(Level<Side::Sell> { Price { 180 + delta}, Amount { 90 }});
                }
        
                update = createEvent<events::OrderBookUpdate>(depth);
                order_book->processAndComplete(update);
            };
            
            SECTION("Without strategy orders")
            {
                // 180  90
                // 170  17
                // 160  80
                // 140  70
                // 130  13
                // 120  60
                // 110  11
                // 102  1
                //
                // 98   1
                // 90   9
                // 80   40
                // 70   7
                // 60   30
                // 40   20
                // 30   3
                // 20   10
                update_order_book();
            
                SECTION("Historical depth")
                {
                    const auto &buy = order_book->historicalDepth().get<Side::Buy>();
                    auto buy_it = buy.begin();
                    CHECK(*buy_it == Level<Side::Buy> { Price { 98 }, Amount { 1 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 90 }, Amount { 9 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 80 }, Amount { 40 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 70 }, Amount { 7 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 60 }, Amount { 30 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 40 }, Amount { 20 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 30 }, Amount { 3 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 20 }, Amount { 10 }});
                    CHECK(buy.size() == 8);
                
                    const auto &sell = order_book->historicalDepth().get<Side::Sell>();
                    auto sell_it = sell.begin();
                    CHECK(*sell_it == Level<Side::Sell> { Price { 102 }, Amount { 1 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 110 }, Amount { 11 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 120 }, Amount { 60 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 130 }, Amount { 13 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 140 }, Amount { 70 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 160 }, Amount { 80 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 170 }, Amount { 17 }});
                    CHECK(sell.size() == 8);
                }
            
                SECTION("Depth")
                {
                    const auto buy = update.depth.get<Side::Buy>();
                    auto buy_it = buy.begin();
                    CHECK(*buy_it == Level<Side::Buy> { Price { 98 }, Amount { 1 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 90 }, Amount { 9 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 80 }, Amount { 40 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 70 }, Amount { 7 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 60 }, Amount { 30 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 40 }, Amount { 20 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 30 }, Amount { 3 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 20 }, Amount { 10 }});
                    CHECK(buy.size() == 8);
                
                    const auto sell = update.depth.get<Side::Sell>();
                    auto sell_it = sell.begin();
                    CHECK(*sell_it == Level<Side::Sell> { Price { 102 }, Amount { 1 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 110 }, Amount { 11 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 120 }, Amount { 60 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 130 }, Amount { 13 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 140 }, Amount { 70 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 160 }, Amount { 80 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 170 }, Amount { 17 }});
                    CHECK(sell.size() == 8);
                }
            }
        
            SECTION("With strategy orders")
            {
                consumer->clear();
            
                // Creating strategy orders
                {
                    place_buy_order(OrderId { 1 }, Price { 99 }, Amount { 42 });
                    place_buy_order(OrderId { 2 }, Price { 98 }, Amount { 42 });
                    place_buy_order(OrderId { 3 }, Price { 90 }, Amount { 42 });
                    place_buy_order(OrderId { 4 }, Price { 80 }, Amount { 42 });
                    place_buy_order(OrderId { 5 }, Price { 70 }, Amount { 42 });
                    place_buy_order(OrderId { 6 }, Price { 50 }, Amount { 42 });
                    place_buy_order(OrderId { 7 }, Price { 30 }, Amount { 42 });
                    place_buy_order(OrderId { 8 }, Price { 20 }, Amount { 42 });
                    place_buy_order(OrderId { 9 }, Price { 10 }, Amount { 42 });
                    place_buy_order(OrderId { 10 }, Price { 99 }, Amount { 21 });
                    place_buy_order(OrderId { 11 }, Price { 98 }, Amount { 21 });
                    place_buy_order(OrderId { 12 }, Price { 90 }, Amount { 21 });
                    place_buy_order(OrderId { 13 }, Price { 80 }, Amount { 21 });
                    place_buy_order(OrderId { 14 }, Price { 70 }, Amount { 21 });
                    place_buy_order(OrderId { 15 }, Price { 50 }, Amount { 21 });
                    place_buy_order(OrderId { 16 }, Price { 30 }, Amount { 21 });
                    place_buy_order(OrderId { 17 }, Price { 20 }, Amount { 21 });
                    place_buy_order(OrderId { 18 }, Price { 10 }, Amount { 21 });
                
                    // Out of order book
                    place_buy_order(OrderId { 19 }, Price { 1 }, Amount { 777 });
                    place_buy_order(OrderId { 20 }, Price { 1 }, Amount { 333 });
                
                    place_sell_order(OrderId { 21 }, Price { 101 }, Amount { 42 });
                    place_sell_order(OrderId { 22 }, Price { 102 }, Amount { 42 });
                    place_sell_order(OrderId { 23 }, Price { 110 }, Amount { 42 });
                    place_sell_order(OrderId { 24 }, Price { 120 }, Amount { 42 });
                    place_sell_order(OrderId { 25 }, Price { 130 }, Amount { 42 });
                    place_sell_order(OrderId { 26 }, Price { 150 }, Amount { 42 });
                    place_sell_order(OrderId { 27 }, Price { 170 }, Amount { 42 });
                    place_sell_order(OrderId { 28 }, Price { 180 }, Amount { 42 });
                    place_sell_order(OrderId { 29 }, Price { 190 }, Amount { 42 });
                    place_sell_order(OrderId { 30 }, Price { 101 }, Amount { 21 });
                    place_sell_order(OrderId { 31 }, Price { 102 }, Amount { 21 });
                    place_sell_order(OrderId { 32 }, Price { 110 }, Amount { 21 });
                    place_sell_order(OrderId { 33 }, Price { 120 }, Amount { 21 });
                    place_sell_order(OrderId { 34 }, Price { 130 }, Amount { 21 });
                    place_sell_order(OrderId { 35 }, Price { 150 }, Amount { 21 });
                    place_sell_order(OrderId { 36 }, Price { 170 }, Amount { 21 });
                    place_sell_order(OrderId { 37 }, Price { 180 }, Amount { 21 });
                    place_sell_order(OrderId { 38 }, Price { 190 }, Amount { 21 });
                
                    // Out of order book
                    place_sell_order(OrderId { 39 }, Price { 199 }, Amount { 777 });
                    place_sell_order(OrderId { 40 }, Price { 199 }, Amount { 333 });
                }
            
                consumer->clear();
            
                // 180  90
                // 170  17
                // 160  80
                // 140  70
                // 130  13
                // 120  60
                // 110  11
                // 102  1
                //
                // 98   1
                // 90   9
                // 80   40
                // 70   7
                // 60   30
                // 40   20
                // 30   3
                // 20   10
                update_order_book();
            
                SECTION("Historical depth")
                {
                    const auto &buy = order_book->historicalDepth().get<Side::Buy>();
                    auto buy_it = buy.begin();
                    CHECK(*buy_it == Level<Side::Buy> { Price { 98 }, Amount { 1 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 90 }, Amount { 9 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 80 }, Amount { 40 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 70 }, Amount { 7 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 60 }, Amount { 30 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 40 }, Amount { 20 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 30 }, Amount { 3 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 20 }, Amount { 10 }});
                    CHECK(buy.size() == 8);
                
                    const auto &sell = order_book->historicalDepth().get<Side::Sell>();
                    auto sell_it = sell.begin();
                    CHECK(*sell_it == Level<Side::Sell> { Price { 102 }, Amount { 1 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 110 }, Amount { 11 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 120 }, Amount { 60 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 130 }, Amount { 13 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 140 }, Amount { 70 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 160 }, Amount { 80 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 170 }, Amount { 17 }});
                    CHECK(sell.size() == 8);
                }
            
                SECTION("Depth")
                {
                    const auto buy = update.depth.get<Side::Buy>();
                    auto buy_it = buy.begin();
                    CHECK(*buy_it == Level<Side::Buy> { Price { 99 }, Amount { 0 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 98 }, Amount { 1 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 90 }, Amount { 9 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 80 }, Amount { 40 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 70 }, Amount { 7 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 60 }, Amount { 30 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 50 }, Amount { 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 40 }, Amount { 20 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 30 }, Amount { 3 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 20 }, Amount { 10 + 42 + 21 }});
                    CHECK(buy.size() == 10);
                
                    const auto sell = update.depth.get<Side::Sell>();
                    auto sell_it = sell.begin();
                    CHECK(*sell_it == Level<Side::Sell> { Price { 101 }, Amount { 0 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 102 }, Amount { 1 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 110 }, Amount { 11 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 120 }, Amount { 60 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 130 }, Amount { 13 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 140 }, Amount { 70 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 150 }, Amount { 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 160 }, Amount { 80 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 170 }, Amount { 17 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 180 }, Amount { 90 + 42 + 21 }});
                    CHECK(sell.size() == 10);
                }
            
                // 200  90
                // 190  17
                // 180  80
                // 160  70
                // 150  13
                // 140  60
                // 130  11
                // 122  1
                //
                // 118  1
                // 110  9
                // 100  40
                // 90   7
                // 80   30
                // 60   20
                // 50   3
                // 40   10
                update_order_book(20);
            
                SECTION("Historical depth")
                {
                    const auto &buy = order_book->historicalDepth().get<Side::Buy>();
                    auto buy_it = buy.begin();
                    CHECK(*buy_it == Level<Side::Buy> { Price { 118 }, Amount { 1 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 110 }, Amount { 9 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 100 }, Amount { 40 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 90 }, Amount { 7 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 80 }, Amount { 30 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 60 }, Amount { 20 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 50 }, Amount { 3 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 40 }, Amount { 10 }});
                    CHECK(buy.size() == 8);
                
                    const auto &sell = order_book->historicalDepth().get<Side::Sell>();
                    auto sell_it = sell.begin();
                    CHECK(*sell_it == Level<Side::Sell> { Price { 122 }, Amount { 1 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 130 }, Amount { 11 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 140 }, Amount { 60 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 150 }, Amount { 13 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 160 }, Amount { 70 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 180 }, Amount { 80 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 190 }, Amount { 17 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 200 }, Amount { 90 }});
                    CHECK(sell.size() == 8);
                }
            
                SECTION("Depth")
                {
                    const auto buy = update.depth.get<Side::Buy>();
                    auto buy_it = buy.begin();
                    CHECK(*buy_it == Level<Side::Buy> { Price { 118 }, Amount { 1 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 110 }, Amount { 9 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 100 }, Amount { 40 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 99 }, Amount { 0 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 98 }, Amount { 0 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 90 }, Amount { 7 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 80 }, Amount { 30 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 70 }, Amount { 0 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 60 }, Amount { 20 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 50 }, Amount { 3 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 40 }, Amount { 10 }});
                    CHECK(buy.size() == 11);
                
                    const auto sell = update.depth.get<Side::Sell>();
                    auto sell_it = sell.begin();
                    CHECK(*sell_it == Level<Side::Sell> { Price { 101 }, Amount { 0 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 102 }, Amount { 0 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 110 }, Amount { 0 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 120 }, Amount { 0 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 122 }, Amount { 1 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 130 }, Amount { 11 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 140 }, Amount { 60 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 150 }, Amount { 13 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 160 }, Amount { 70 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 170 }, Amount { 0 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 180 }, Amount { 80 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 190 }, Amount { 17 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 199 }, Amount { 0 + 777 + 333 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 200 }, Amount { 90 }});
                    CHECK(sell.size() == 14);
                }
            
                // 160 90
                // 150 17
                // 140 80
                // 120 70
                // 110 13
                // 100 60
                // 90  11
                // 82  1
                //
                // 78  1
                // 70  9
                // 60  40
                // 50  7
                // 40  30
                // 20  20
                // 10  3
                // 0   10
                update_order_book(-20);
    
                SECTION("Historical depth")
                {
                    const auto &buy = order_book->historicalDepth().get<Side::Buy>();
                    auto buy_it = buy.begin();
                    CHECK(*buy_it == Level<Side::Buy> { Price { 78 }, Amount { 1 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 70 }, Amount { 9 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 60 }, Amount { 40 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 50 }, Amount { 7 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 40 }, Amount { 30 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 20 }, Amount { 20 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 10 }, Amount { 3 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 0 }, Amount { 10 }});
                    CHECK(buy.size() == 8);
        
                    const auto &sell = order_book->historicalDepth().get<Side::Sell>();
                    auto sell_it = sell.begin();
                    CHECK(*sell_it == Level<Side::Sell> { Price { 82 }, Amount { 1 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 90 }, Amount { 11 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 100 }, Amount { 60 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 110 }, Amount { 13 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 120 }, Amount { 70 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 140 }, Amount { 80 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 150 }, Amount { 17 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 160 }, Amount { 90 }});
                    CHECK(sell.size() == 8);
                }
    
                SECTION("Depth")
                {
                    const auto buy = update.depth.get<Side::Buy>();
                    auto buy_it = buy.begin();
                    CHECK(*buy_it == Level<Side::Buy> { Price { 99 }, Amount { 0 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 98 }, Amount { 0 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 90 }, Amount { 0 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 80 }, Amount { 0 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 78 }, Amount { 1 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 70 }, Amount { 9 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 60 }, Amount { 40 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 50 }, Amount { 7 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 40 }, Amount { 30 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 30 }, Amount { 0 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 20 }, Amount { 20 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 10 }, Amount { 3 + 42 + 21 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 1 }, Amount { 0 + 777 + 333 }});
                    ++buy_it;
                    CHECK(*buy_it == Level<Side::Buy> { Price { 0 }, Amount { 10 }});
                    CHECK(buy.size() == 14);
                    
                    const auto sell = update.depth.get<Side::Sell>();
                    auto sell_it = sell.begin();
                    CHECK(*sell_it == Level<Side::Sell> { Price { 82 }, Amount { 1 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 90 }, Amount { 11 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 100 }, Amount { 60 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 101 }, Amount { 0 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 102 }, Amount { 0 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 110 }, Amount { 13 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 120 }, Amount { 70 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 130 }, Amount { 0 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 140 }, Amount { 80 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 150 }, Amount { 17 + 42 + 21 }});
                    ++sell_it;
                    CHECK(*sell_it == Level<Side::Sell> { Price { 160 }, Amount { 90 }});
                    CHECK(sell.size() == 11);
                }
            }
        }
        
        SECTION("Moving and (un)freezing orders")
        {
            SECTION("Order was in spread")
            {
                place_buy_order(OrderId{ 1 }, Price { 90 }, Amount { 10 });
                place_buy_order(OrderId{ 2 }, Price { 90 }, Amount { 5 });
    
                place_sell_order(OrderId{ 3 }, Price { 110 }, Amount { 10 });
                place_sell_order(OrderId{ 4 }, Price { 110 }, Amount { 5 });
    
                REQUIRE(consumer->freezeOrderEvents().empty());
    
                depth.get<Side::Buy>().push_front(Level<Side::Buy> { Price { 90 }, Amount { 100 }});
                depth.get<Side::Sell>().push_front(Level<Side::Sell> { Price { 110 }, Amount { 100 }});
                
                consumer->clear();
    
                update = createEvent<events::OrderBookUpdate>(depth);
                order_book->processAndComplete(update);
                
                CHECK(consumer->moveOrderEvents().empty());
                CHECK(consumer->freezeOrderEvents().empty());
            }
            
            SECTION("Order was on empty price level")
            {
                place_buy_order(OrderId{ 1 }, Price { 70 }, Amount { 10 });
                place_buy_order(OrderId{ 2 }, Price { 70 }, Amount { 5 });
    
                place_sell_order(OrderId{ 3 }, Price { 130 }, Amount { 10 });
                place_sell_order(OrderId{ 4 }, Price { 130 }, Amount { 5 });
    
                REQUIRE(consumer->freezeOrderEvents().empty());
    
                depth.get<Side::Buy>().insert(
                    ++depth.get<Side::Buy>().begin(), Level<Side::Buy> { Price { 70 }, Amount { 100 }});
                depth.get<Side::Sell>().insert(
                    ++depth.get<Side::Sell>().begin(), Level<Side::Sell> { Price { 130 }, Amount { 100 }});
    
                consumer->clear();
    
                update = createEvent<events::OrderBookUpdate>(depth);
                order_book->processAndComplete(update);
    
                CHECK(consumer->moveOrderEvents().empty());
                CHECK(consumer->freezeOrderEvents().empty());
            }
            
            SECTION("Order was on not empty price level")
            {
                place_buy_order(OrderId{ 1 }, Price { 80 }, Amount { 10 });
                place_buy_order(OrderId{ 2 }, Price { 80 }, Amount { 5 });
    
                place_sell_order(OrderId{ 3 }, Price { 120 }, Amount { 10 });
                place_sell_order(OrderId{ 4 }, Price { 120 }, Amount { 5 });
    
                REQUIRE(consumer->freezeOrderEvents().empty());
    
                SECTION("Increasing price level volume")
                {
                    depth.get<Side::Buy>().front().updateVolume(Amount { 100 });
                    depth.get<Side::Sell>().front().updateVolume(Amount { 100 });
        
                    consumer->clear();
        
                    update = createEvent<events::OrderBookUpdate>(depth);
                    order_book->processAndComplete(update);
        
                    CHECK(consumer->moveOrderEvents().empty());
                    CHECK(consumer->freezeOrderEvents().empty());
                }
    
                SECTION("Decreasing price level volume")
                {
                    {
                        depth.get<Side::Buy>().front().updateVolume(Amount { 6 });
                        depth.get<Side::Sell>().front().updateVolume(Amount { 8 });
        
                        consumer->clear();
        
                        update = createEvent<events::OrderBookUpdate>(depth);
                        order_book->processAndComplete(update);
        
                        const auto &move_order_events = consumer->moveOrderEvents();
                        auto move = move_order_events.begin();
                        CHECK(move->client_order_id == OrderId { 1 });
                        CHECK(move->volume_before == Amount { 6 });
                        ++move;
                        CHECK(move->client_order_id == OrderId { 2 });
                        CHECK(move->volume_before == Amount { 6 });
                        ++move;
                        CHECK(move->client_order_id == OrderId { 3 });
                        CHECK(move->volume_before == Amount { 8 });
                        ++move;
                        CHECK(move->client_order_id == OrderId { 4 });
                        CHECK(move->volume_before == Amount { 8 });
                        CHECK(move_order_events.size() == 4);
        
                        CHECK(consumer->freezeOrderEvents().empty());
                    }
    
                    {
                        depth.get<Side::Buy>().front().updateVolume(Amount { 60 });
                        depth.get<Side::Sell>().front().updateVolume(Amount { 80 });
    
                        update = createEvent<events::OrderBookUpdate>(depth);
                        order_book->processAndComplete(update);
    
                        depth.get<Side::Buy>().front().updateVolume(Amount { 54 });
                        depth.get<Side::Sell>().front().updateVolume(Amount { 72 });
    
                        consumer->clear();
                        
                        update = createEvent<events::OrderBookUpdate>(depth);
                        order_book->processAndComplete(update);
        
                        const auto &move_order_events = consumer->moveOrderEvents();
                        auto move = move_order_events.begin();
                        CHECK(move->client_order_id == OrderId { 1 });
                        CHECK(move->volume_before == Amount { 6 * ((1.0 + 54.0 / 60.0) / 2) }); // 6 - 5%
                        ++move;
                        CHECK(move->client_order_id == OrderId { 2 });
                        CHECK(move->volume_before == Amount { 6 * ((1.0 + 54.0 / 60.0) / 2) });
                        ++move;
                        CHECK(move->client_order_id == OrderId { 3 });
                        CHECK(move->volume_before == Amount { 8 * ((1.0 + 54.0 / 60.0) / 2) }); // 8 - 5%
                        ++move;
                        CHECK(move->client_order_id == OrderId { 4 });
                        CHECK(move->volume_before == Amount { 8 * ((1.0 + 54.0 / 60.0) / 2) });
                        CHECK(move_order_events.size() == 4);
        
                        CHECK(consumer->freezeOrderEvents().empty());
                    }
                }
            }
    
            SECTION("Order was out of order book")
            {
                SECTION("Order was freezed while placing")
                {
                    place_buy_order(OrderId { 1 }, Price { 1 }, Amount { 10 });
                    place_buy_order(OrderId { 2 }, Price { 1 }, Amount { 5 });
        
                    place_sell_order(OrderId { 3 }, Price { 199 }, Amount { 10 });
                    place_sell_order(OrderId { 4 }, Price { 199 }, Amount { 5 });
        
                    const auto &freeze_order_events = consumer->freezeOrderEvents();
                    auto freeze = freeze_order_events.begin();
                    REQUIRE(freeze->client_order_id == OrderId { 1 });
                    ++freeze;
                    REQUIRE(freeze->client_order_id == OrderId { 2 });
                    ++freeze;
                    REQUIRE(freeze->client_order_id == OrderId { 3 });
                    ++freeze;
                    REQUIRE(freeze->client_order_id == OrderId { 4 });
                    REQUIRE(freeze_order_events.size() == 4);
        
                    depth.get<Side::Buy>().insert(
                        depth.get<Side::Buy>().end(), Level<Side::Buy> { Price { 1 }, Amount { 100 }});
                    depth.get<Side::Sell>().insert(
                        depth.get<Side::Sell>().end(), Level<Side::Sell> { Price { 199 }, Amount { 100 }});
    
                    consumer->clear();
                    
                    update = createEvent<events::OrderBookUpdate>(depth);
                    order_book->processAndComplete(update);
        
                    const auto &unfreeze_order_events = consumer->unfreezeOrderEvents();
                    auto unfreeze = unfreeze_order_events.begin();
                    CHECK(unfreeze->client_order_id == OrderId { 1 });
                    ++unfreeze;
                    CHECK(unfreeze->client_order_id == OrderId { 2 });
                    ++unfreeze;
                    CHECK(unfreeze->client_order_id == OrderId { 3 });
                    ++unfreeze;
                    CHECK(unfreeze->client_order_id == OrderId { 4 });
                    CHECK(unfreeze_order_events.size() == 4);
        
                    const auto &move_order_events = consumer->moveOrderEvents();
                    auto move = move_order_events.begin();
                    CHECK(move->client_order_id == OrderId { 1 });
                    CHECK(move->volume_before == Amount { 100 });
                    ++move;
                    CHECK(move->client_order_id == OrderId { 2 });
                    CHECK(move->volume_before == Amount { 100 });
                    ++move;
                    CHECK(move->client_order_id == OrderId { 3 });
                    CHECK(move->volume_before == Amount { 100 });
                    ++move;
                    CHECK(move->client_order_id == OrderId { 4 });
                    CHECK(move->volume_before == Amount { 100 });
                    CHECK(move_order_events.size() == 4);
    
                    CHECK(consumer->freezeOrderEvents().empty());
                }
                
                SECTION("Order was freezed after order book update")
                {
                    place_buy_order(OrderId { 1 }, Price { 5 }, Amount { 10 });
                    place_buy_order(OrderId { 2 }, Price { 5 }, Amount { 5 });
    
                    place_sell_order(OrderId { 3 }, Price { 195 }, Amount { 10 });
                    place_sell_order(OrderId { 4 }, Price { 195 }, Amount { 5 });
    
                    REQUIRE(consumer->freezeOrderEvents().empty());
    
                    consumer->clear();
    
                    depth.get<Side::Buy>().pop_back();
                    depth.get<Side::Sell>().pop_back();
    
                    update = createEvent<events::OrderBookUpdate>(depth);
                    order_book->processAndComplete(update);
                    
                    const auto &freeze_order_events = consumer->freezeOrderEvents();
                    auto freeze = freeze_order_events.begin();
                    REQUIRE(freeze->client_order_id == OrderId { 1 });
                    ++freeze;
                    REQUIRE(freeze->client_order_id == OrderId { 2 });
                    ++freeze;
                    REQUIRE(freeze->client_order_id == OrderId { 3 });
                    ++freeze;
                    REQUIRE(freeze->client_order_id == OrderId { 4 });
                    REQUIRE(freeze_order_events.size() == 4);
    
                    REQUIRE(consumer->moveOrderEvents().empty());
    
                    consumer->clear();
    
                    SECTION("Must unfreeze")
                    {
                        depth.get<Side::Buy>().insert(
                            depth.get<Side::Buy>().end(), Level<Side::Buy> { Price { 5 }, Amount { 100 }});
                        depth.get<Side::Sell>().insert(
                            depth.get<Side::Sell>().end(), Level<Side::Sell> { Price { 195 }, Amount { 100 }});
        
                        update = createEvent<events::OrderBookUpdate>(depth);
                        order_book->processAndComplete(update);
        
                        const auto &unfreeze_order_events = consumer->unfreezeOrderEvents();
                        auto unfreeze = unfreeze_order_events.begin();
                        CHECK(unfreeze->client_order_id == OrderId { 1 });
                        ++unfreeze;
                        CHECK(unfreeze->client_order_id == OrderId { 2 });
                        ++unfreeze;
                        CHECK(unfreeze->client_order_id == OrderId { 3 });
                        ++unfreeze;
                        CHECK(unfreeze->client_order_id == OrderId { 4 });
                        CHECK(unfreeze_order_events.size() == 4);
        
                        // Price level volume was increased, must be previous volume before
                        const auto &move_order_events = consumer->moveOrderEvents();
                        auto move = move_order_events.begin();
                        CHECK(move->client_order_id == OrderId { 1 });
                        CHECK(move->volume_before == Amount { 0.5 });
                        ++move;
                        CHECK(move->client_order_id == OrderId { 2 });
                        CHECK(move->volume_before == Amount { 0.5 });
                        ++move;
                        CHECK(move->client_order_id == OrderId { 3 });
                        CHECK(move->volume_before == Amount { 19.5 });
                        ++move;
                        CHECK(move->client_order_id == OrderId { 4 });
                        CHECK(move->volume_before == Amount { 19.5 });
                        CHECK(move_order_events.size() == 4);
        
                        CHECK(consumer->freezeOrderEvents().empty());
                    }
                    
                    SECTION("Must stay freezed")
                    {
                        depth.get<Side::Buy>().insert(
                            depth.get<Side::Buy>().end(), Level<Side::Buy> { Price { 10 }, Amount { 100 }});
                        depth.get<Side::Sell>().insert(
                            depth.get<Side::Sell>().end(), Level<Side::Sell> { Price { 190 }, Amount { 100 }});
    
                        update = createEvent<events::OrderBookUpdate>(depth);
                        order_book->processAndComplete(update);
    
                        CHECK(consumer->freezeOrderEvents().empty());
                        CHECK(consumer->unfreezeOrderEvents().empty());
                        CHECK(consumer->moveOrderEvents().empty());
                    }
                }
            }
        }
    }
}
