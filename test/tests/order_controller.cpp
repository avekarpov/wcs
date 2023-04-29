#include <catch2/catch.hpp>

#include <wcs/order_controller.hpp>
#include <wcs/order_book.hpp>

#include "../spies/consumer.hpp"
#include "../utilits/side_section.hpp"

using namespace wcs;

using spies::Consumer;

template <Side S>
void marketOrders(std::shared_ptr<Consumer> consumer, std::shared_ptr<OrderController<Consumer>> order_controller)
{
    const auto order_manager = order_controller->orderManager();
    const auto market_orders = order_manager->marketOrders().get<S>();
    
    REQUIRE(market_orders);
    REQUIRE(market_orders->empty());
    
    SECTION("Place/Cancel order")
    {
        {
            consumer->clear();
    
            order_controller->process(
                EventBuilder::build<events::PlaceOrder<S, OrderType::Market>>(OrderId { 1 }, Amount { 1 }));
            order_controller->process(
                EventBuilder::build<events::PlaceOrder<S, OrderType::Market>>(OrderId { 2 }, Amount { 2 }));
            order_controller->process(
                EventBuilder::build<events::PlaceOrder<S, OrderType::Market>>(OrderId { 3 }, Amount { 3 }));
            
            const auto &order_update_events = consumer->orderUpdateEvents();
            auto order_update = order_update_events.begin();
            CHECK(std::get<events::OrderUpdate<OrderStatus::Placed>>(*order_update).client_order_id == OrderId { 1 });
            ++order_update;
            CHECK(std::get<events::OrderUpdate<OrderStatus::Placed>>(*order_update).client_order_id == OrderId { 2 });
            ++order_update;
            CHECK(std::get<events::OrderUpdate<OrderStatus::Placed>>(*order_update).client_order_id == OrderId { 3 });
            CHECK(order_update_events.size() == 3);
        }
        
            auto order = market_orders->begin();
            CHECK(order->id() == 1);
            CHECK(order->amount() == Amount { 1 });
            CHECK(order->status() == OrderStatus::Placed);
            ++order;
            CHECK(order->id() == 2);
            CHECK(order->amount() == Amount { 2 });
            CHECK(order->status() == OrderStatus::Placed);
            ++order;
            CHECK(order->id() == 3);
            CHECK(order->amount() == Amount { 3 });
            CHECK(order->status() == OrderStatus::Placed);
            CHECK(market_orders->size() == 3);
            
        {
            consumer->clear();
    
            order_controller->process(EventBuilder::build<events::CancelOrder>(OrderId{ 1 }));
            order_controller->process(EventBuilder::build<events::CancelOrder>(OrderId{ 2 }));
            order_controller->process(EventBuilder::build<events::CancelOrder>(OrderId{ 3 }));
            
            const auto &order_update_events = consumer->orderUpdateEvents();
            auto order_update = order_update_events.begin();
            CHECK(std::get<events::OrderUpdate<OrderStatus::Canceled>>(*order_update).client_order_id == OrderId { 1 });
            ++order_update;
            CHECK(std::get<events::OrderUpdate<OrderStatus::Canceled>>(*order_update).client_order_id == OrderId { 2 });
            ++order_update;
            CHECK(std::get<events::OrderUpdate<OrderStatus::Canceled>>(*order_update).client_order_id == OrderId { 3 });
            CHECK(order_update_events.size() == 3);
        }
        
        REQUIRE(market_orders->empty());
    }
    
    SECTION("Fill order")
    {
        consumer->clear();
        
        order_controller->process(
            EventBuilder::build<events::PlaceOrder<S, OrderType::Market>>(OrderId { 1 }, Amount { 100 }));
        const auto &order = market_orders->front();
        
        CHECK(order.filledAmount() == Amount { 0 });
        CHECK(order.status() == OrderStatus::Placed);
        
        order_controller->process(EventBuilder::build<events::FillOrder>(OrderId { 1 }, Amount { 10 }));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Partially>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 1 });
            CHECK(last_order_update.amount == Amount { 10 });
        }
    
        CHECK(order.filledAmount() == Amount { 10 });
        CHECK(order.status() == OrderStatus::Partially);
    
        order_controller->process(EventBuilder::build<events::FillOrder>(OrderId { 1 }, Amount { 20 }));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Partially>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 1 });
            CHECK(last_order_update.amount == Amount { 30 });
        }
    
        CHECK(order.filledAmount() == Amount { 30 });
        CHECK(order.status() == OrderStatus::Partially);
    
        order_controller->process(EventBuilder::build<events::FillOrder>(OrderId { 1 }, Amount { 30 }));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Partially>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 1 });
            CHECK(last_order_update.amount == Amount { 60 });
        }
    
        CHECK(order.filledAmount() == Amount { 60 });
        CHECK(order.status() == OrderStatus::Partially);
    
        order_controller->process(EventBuilder::build<events::FillOrder>(OrderId { 1 }, Amount { 40 }));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Filled>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 1 });
            CHECK(last_order_update.amount == Amount { 100 });
        }
        
        REQUIRE(market_orders->empty());
    }
}

template <Side S>
void limitOrders(
    std::shared_ptr<Consumer> consumer,
    std::shared_ptr<OrderController<Consumer>> order_controller,
    std::shared_ptr<OrderBook<Consumer>> order_book)
{
    const auto order_manager = order_controller->orderManager();
    const auto limit_orders = order_manager->limitOrders().get<S>();
    
    REQUIRE(limit_orders);
    REQUIRE(limit_orders->empty());
    
    SECTION("Place/Cancel order")
    {
        const auto is_sorted_by_price = [](const auto &orders)
        {
            Price previous = orders->begin()->price();
            for (auto it = ++orders->begin(); it != orders->end(); ++it)
            {
                if (utilits::sideLess<S>(previous, it->price())) {
                    return false;
                }
                
                previous = it->price();
            }
            
            return true;
        };
        
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 1 },
            Price { 100 },
            Amount { 1 }
        ));
    
        {
            const auto &last_order_update = 
                std::get<events::OrderUpdate<OrderStatus::Placed>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 1 });
        }
    
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 2 },
            Price { 50 },
            Amount { 2 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Placed>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 2 });
        }
        
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 3 },
            Price { 150 },
            Amount { 3 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Placed>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 3 });
        }
    
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 4 },
            Price { 75 },
            Amount { 4 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Placed>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 4 });
        }
    
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 5 },
            Price { 125 },
            Amount { 5 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Placed>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 5 });
        }
        
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 6 },
            Price { 100 },
            Amount { 6 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Placed>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 6 });
        }
    
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 7 },
            Price { 50 },
            Amount { 7 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Placed>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 7 });
        }
    
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 8 },
            Price { 150 },
            Amount { 8 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Placed>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 8 });
        }
    
        CHECK(is_sorted_by_price(limit_orders));
    
        CHECK(limit_orders->size() == 8);
    
        order_controller->process(EventBuilder::build<events::CancelOrder>(
            OrderId { 1 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Canceled>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 1 });
        }
        
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::CancelOrder>(
            OrderId { 2 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Canceled>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 2 });
        }
        
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::CancelOrder>(
            OrderId { 3 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Canceled>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 3 });
        }
        
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::CancelOrder>(
            OrderId { 4 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Canceled>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 4 });
        }
        
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::CancelOrder>(
            OrderId { 5 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Canceled>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 5 });
        }
        
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::CancelOrder>(
            OrderId { 6 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Canceled>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 6 });
        }
        
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::CancelOrder>(
            OrderId { 7 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Canceled>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 7 });
        }
        
        CHECK(is_sorted_by_price(limit_orders));
    
        order_controller->process(EventBuilder::build<events::CancelOrder>(
            OrderId { 8 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Canceled>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 8 });
        }
    
        CHECK(limit_orders->empty());
    }
    
    SECTION("Fill order")
    {
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 1 },
            Price { 99 },
            Amount { 100 }
        ));
        const auto &order = limit_orders->front();
    
        CHECK(order.filledAmount() == Amount { 0 });
        CHECK(order.status() == OrderStatus::Placed);
        
        order_controller->process(EventBuilder::build<events::FillOrder>(
           OrderId { 1 },
           Amount { 10 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Partially>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 1 });
            CHECK(last_order_update.amount == Amount { 10 });
        }
    
        CHECK(order.filledAmount() == Amount { 10 });
    
        order_controller->process(EventBuilder::build<events::FillOrder>(
            OrderId { 1 },
            Amount { 20 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Partially>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 1 });
            CHECK(last_order_update.amount == Amount { 30 });
        }
    
        REQUIRE(limit_orders->front().filledAmount() == Amount { 30 });
    
        order_controller->process(EventBuilder::build<events::FillOrder>(
            OrderId { 1 },
            Amount { 30 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Partially>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 1 });
            CHECK(last_order_update.amount == Amount { 60 });
        }
    
        REQUIRE(limit_orders->front().filledAmount() == Amount { 60 });
    
        order_controller->process(EventBuilder::build<events::FillOrder>(
            OrderId { 1 },
            Amount { 40 }
        ));
    
        {
            const auto &last_order_update =
                std::get<events::OrderUpdate<OrderStatus::Filled>>(consumer->orderUpdateEvents().back());
            CHECK(last_order_update.client_order_id == OrderId { 1 });
        }
    
        REQUIRE(limit_orders->empty());
    }
    
    SECTION("Shift")
    {
        SidePair<Depth> depth;
        events::OrderBookUpdate update { .depth = depth };
    
        depth.get<S>().push_back(Level<S> { Price { 99 }, Amount { 10 } });
        update = EventBuilder::build<events::OrderBookUpdate>(depth);
        order_book->processAndComplete(update);
        
        order_controller->process(EventBuilder::build<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 1 },
            Price { 99 },
            Amount { 100 }
        ));
        const auto &order = limit_orders->front();
    
        REQUIRE(order.volumeBefore() == Amount { 10 });
        CHECK(order.status() == OrderStatus::Placed);
        
        order_controller->process(EventBuilder::build<events::ShiftOrder>(
            OrderId { 1 },
            Amount { 10 }
        ));
        
        CHECK(!order.volumeBefore());
    }
}

TEST_CASE("OrderController")
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
            buy.push_back(Level<Side::Buy> { Price { 0 }, Amount { 0 }});
            
            auto &sell = depth.get<Side::Sell>();
            sell.push_back(Level<Side::Sell> { Price { std::numeric_limits<double>::max() }, Amount { 0 }});
        }
        
        update = EventBuilder::build<events::OrderBookUpdate>(depth);
        order_book->processAndComplete(update);
    }
    
    SIDE_SECTION("Market orders", marketOrders, consumer, order_controller);
    
    SIDE_SECTION("Limit orders", limitOrders, consumer, order_controller, order_book);
}
