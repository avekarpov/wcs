#include <catch2/catch.hpp>

#include <wcs/order_controller.hpp>
#include <wcs/order_book.hpp>

#include "../spies/consumer.hpp"
#include "../utilits/side_section.hpp"
#include "../utilits/event_builder.hpp"

using namespace wcs;

using spies::Consumer;

// TODO: add second order for price level
// TODO: test shifting order
// TODO: refactoring

template <Side S>
void marketOrders(std::shared_ptr<Consumer> consumer, std::shared_ptr<OrderController<Consumer>> order_controller)
{
    const auto order_manager = order_controller->orderManager();
    const auto market_orders = order_manager->marketOrders().get<S>();
    
    REQUIRE(market_orders);
    REQUIRE(market_orders->empty());
    
    SECTION("Place/Cancel order")
    {
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Market>>(OrderId { 1 }, Amount { 1 }));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 1);
        },
        consumer->orderUpdateEvents().back());
    
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Market>>(OrderId { 2 }, Amount { 2 }));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 2);
        },
        consumer->orderUpdateEvents().back());
    
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Market>>(OrderId { 3 }, Amount { 3 }));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 3);
        },
        consumer->orderUpdateEvents().back());
        
        auto orders_it = market_orders->begin();
        REQUIRE(orders_it->id() == 1);
        REQUIRE(orders_it->amount() == Amount{ 1 });
        
        ++orders_it;
        REQUIRE(orders_it->id() == 2);
        REQUIRE(orders_it->amount() == Amount{ 2 });
        
        ++orders_it;
        REQUIRE(orders_it->id() == 3);
        REQUIRE(orders_it->amount() == Amount{ 3 });
        
        REQUIRE(market_orders->size() == 3);
        
        order_controller->process(createEvent<events::CancelOrder>(OrderId{ 1 }));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 1);
        },
        consumer->orderUpdateEvents().back());
        
        orders_it = market_orders->begin();
        REQUIRE(orders_it->id() == 2);
        REQUIRE(orders_it->amount() == Amount{ 2 });
    
        order_controller->process(createEvent<events::CancelOrder>(OrderId{ 2 }));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 2);
        },
        consumer->orderUpdateEvents().back());
        
        orders_it = market_orders->begin();
        REQUIRE(orders_it->id() == 3);
        REQUIRE(orders_it->amount() == Amount{ 3 });
    
        order_controller->process(createEvent<events::CancelOrder>(OrderId{ 3 }));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 3);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(market_orders->empty());
    }
    
    SECTION("Fill order")
    {
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Market>>(OrderId { 1 }, Amount { 100 }));
        
        REQUIRE(market_orders->front().filledAmount() == Amount { 0 });
        
        order_controller->process(createEvent<events::FillOrder>(OrderId { 1 }, Amount { 10 }));
    
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            if constexpr (OS == OrderStatus::Partially) {
                REQUIRE(event.client_order_id == 1);
                REQUIRE(event.amount == Amount { 10 });
            }
            else {
                FAIL();
            }
        },
        consumer->orderUpdateEvents().back());
    
        REQUIRE(market_orders->front().filledAmount() == Amount { 10 });
    
        order_controller->process(createEvent<events::FillOrder>(OrderId { 1 }, Amount { 20 }));
    
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            if constexpr (OS == OrderStatus::Partially) {
                REQUIRE(event.client_order_id == 1);
                REQUIRE(event.amount == Amount { 30 });
            }
            else {
                FAIL();
            }
        },
        consumer->orderUpdateEvents().back());
    
        REQUIRE(market_orders->front().filledAmount() == Amount { 30 });
    
        order_controller->process(createEvent<events::FillOrder>(OrderId { 1 }, Amount { 30 }));
    
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            if constexpr (OS == OrderStatus::Partially) {
                REQUIRE(event.client_order_id == 1);
                REQUIRE(event.amount == Amount { 60 });
            }
            else {
                FAIL();
            }
        },
        consumer->orderUpdateEvents().back());
    
        REQUIRE(market_orders->front().filledAmount() == Amount { 60 });
    
        order_controller->process(createEvent<events::FillOrder>(OrderId { 1 }, Amount { 40 }));
    
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            if constexpr (OS == OrderStatus::Filled) {
                REQUIRE(event.client_order_id == 1);
            }
            else {
                FAIL();
            }
        },
        consumer->orderUpdateEvents().back());
    
        REQUIRE(market_orders->empty());
    }
}

template <Side S>
void limitOrders(std::shared_ptr<Consumer> consumer, std::shared_ptr<OrderController<Consumer>> order_controller)
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
        
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 1 },
            Price { 100 },
            Amount { 1 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 1);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 2 },
            Price { 50 },
            Amount { 2 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 2);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 3 },
            Price { 150 },
            Amount { 3 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 3);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 4 },
            Price { 75 },
            Amount { 4 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 4);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 5 },
            Price { 125 },
            Amount { 5 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 5);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 6 },
            Price { 100 },
            Amount { 6 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 6);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 7 },
            Price { 50 },
            Amount { 7 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 7);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 8 },
            Price { 150 },
            Amount { 8 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Placed);
            REQUIRE(event.client_order_id == 8);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        REQUIRE(limit_orders->size() == 8);
    
        order_controller->process(createEvent<events::CancelOrder>(
            OrderId { 1 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 1);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::CancelOrder>(
            OrderId { 2 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 2);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::CancelOrder>(
            OrderId { 3 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 3);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::CancelOrder>(
            OrderId { 4 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 4);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::CancelOrder>(
            OrderId { 5 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 5);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::CancelOrder>(
            OrderId { 6 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 6);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::CancelOrder>(
            OrderId { 7 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 7);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
    
        order_controller->process(createEvent<events::CancelOrder>(
            OrderId { 8 }
        ));
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 8);
        },
        consumer->orderUpdateEvents().back());
        
        REQUIRE(limit_orders->empty());
    }
    
    SECTION("Fill order")
    {
        order_controller->process(createEvent<events::PlaceOrder<S, OrderType::Limit>>(
            OrderId { 1 },
            Price { 99 },
            Amount { 100 }
        ));
    
        REQUIRE(limit_orders->front().filledAmount() == Amount { 0 });
        
        order_controller->process(createEvent<events::FillOrder>(
           OrderId { 1 },
           Amount { 10 }
        ));
    
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            if constexpr (OS == OrderStatus::Partially) {
                REQUIRE(event.client_order_id == 1);
                REQUIRE(event.amount == Amount { 10 });
            }
            else {
                FAIL();
            }
        },
        consumer->orderUpdateEvents().back());
    
        REQUIRE(limit_orders->front().filledAmount() == Amount { 10 });
    
        order_controller->process(createEvent<events::FillOrder>(
            OrderId { 1 },
            Amount { 20 }
        ));
    
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            if constexpr (OS == OrderStatus::Partially) {
                REQUIRE(event.client_order_id == 1);
                REQUIRE(event.amount == Amount { 30 });
            }
            else {
                FAIL();
            }
        },
        consumer->orderUpdateEvents().back());
    
        REQUIRE(limit_orders->front().filledAmount() == Amount { 30 });
    
        order_controller->process(createEvent<events::FillOrder>(
            OrderId { 1 },
            Amount { 30 }
        ));
    
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            if constexpr (OS == OrderStatus::Partially) {
                REQUIRE(event.client_order_id == 1);
                REQUIRE(event.amount == Amount { 60 });
            }
            else {
                FAIL();
            }
        },
        consumer->orderUpdateEvents().back());
    
        REQUIRE(limit_orders->front().filledAmount() == Amount { 60 });
    
        order_controller->process(createEvent<events::FillOrder>(
            OrderId { 1 },
            Amount { 40 }
        ));
    
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            if constexpr (OS == OrderStatus::Filled) {
                REQUIRE(event.client_order_id == 1);
            }
            else {
                FAIL();
            }
        },
        consumer->orderUpdateEvents().back());
    
        REQUIRE(limit_orders->empty());
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
        
        update = createEvent<events::OrderBookUpdate>(depth);
        order_book->processAndComplete(update);
    }
    
    SIDE_SECTION("Market orders", marketOrders, consumer, order_controller);
    
    SIDE_SECTION("Limit orders", limitOrders, consumer, order_controller);
}
