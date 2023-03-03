#include <catch2/catch.hpp>

#include <wcs/order_controller.hpp>

#include "../spies/consumer.hpp"
#include "../utilits/side_section.hpp"

using namespace wcs;

using spies::Consumer;

template <Side S>
void MarketOrders(std::shared_ptr<Consumer> consumer, OrderController<Consumer> &order_controller)
{
    const auto order_manager = order_controller.orderManager();
    const auto market_orders = order_manager->marketOrders().get<S>();
    
    REQUIRE(market_orders);
    REQUIRE(market_orders->empty());
    
    SECTION("Place/Cancel order")
    {
        order_controller.process(events::PlaceOrder<S, OrderType::Market>
        {
            Ts { 1 },
            EventId { 1 },
            OrderId { 1 },
            Amount { 1 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 1);
        },
        consumer->orderUpdates().back());
        
        order_controller.process(events::PlaceOrder<S, OrderType::Market>
        {
            Ts { 2 },
            EventId { 2 },
            OrderId { 2 },
            Amount { 2 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 2);
        },
        consumer->orderUpdates().back());
        
        order_controller.process(events::PlaceOrder<S, OrderType::Market>
        {
            Ts { 3 },
            EventId { 3 },
            OrderId { 3 },
            Amount { 3 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 3);
        },
        consumer->orderUpdates().back());
        
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
        
        order_controller.process(events::CancelOrder
        {
            Ts { 4 },
            EventId { 4 },
            OrderId{ 1 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 1);
        },
        consumer->orderUpdates().back());
        
        orders_it = market_orders->begin();
        REQUIRE(orders_it->id() == 2);
        REQUIRE(orders_it->amount() == Amount{ 2 });
        
        order_controller.process(events::CancelOrder
        {
            Ts { 5 },
            EventId { 5 },
            OrderId{ 2 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 2);
        },
        consumer->orderUpdates().back());
        
        orders_it = market_orders->begin();
        REQUIRE(orders_it->id() == 3);
        REQUIRE(orders_it->amount() == Amount{ 3 });
        
        order_controller.process(events::CancelOrder
        {
            Ts { 6 },
            EventId { 6 },
            OrderId{ 3 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 3);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(market_orders->empty());
    }
    
    SECTION("Fill order")
    {
        order_controller.process(events::PlaceOrder<S, OrderType::Market>
        {
            Ts { 1 },
            EventId { 1 },
            OrderId { 1 },
            Amount { 100 }
        });
        
        REQUIRE(market_orders->front().filledAmount() == Amount { 0 });
        
        order_controller.process(events::FillOrder
        {
            Ts { 2 },
            EventId { 2 },
            OrderId { 1 },
            Amount { 10 }
        });
    
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
        consumer->orderUpdates().back());
    
        REQUIRE(market_orders->front().filledAmount() == Amount { 10 });
    
        order_controller.process(events::FillOrder
        {
            Ts { 3 },
            EventId { 3 },
            OrderId { 1 },
            Amount { 20 }
        });
    
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
        consumer->orderUpdates().back());
    
        REQUIRE(market_orders->front().filledAmount() == Amount { 30 });
    
        order_controller.process(events::FillOrder
        {
            Ts { 4 },
            EventId { 4 },
            OrderId { 1 },
            Amount { 30 }
        });
    
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
        consumer->orderUpdates().back());
    
        REQUIRE(market_orders->front().filledAmount() == Amount { 60 });
    
        order_controller.process(events::FillOrder
        {
            Ts { 5 },
            EventId { 5 },
            OrderId { 1 },
            Amount { 40 }
        });
    
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            if constexpr (OS == OrderStatus::Filled) {
                REQUIRE(event.client_order_id == 1);
            }
            else {
                FAIL();
            }
        },
        consumer->orderUpdates().back());
    
        REQUIRE(market_orders->empty());
    }
}

template <Side S>
void LimitOrders(std::shared_ptr<Consumer> consumer, OrderController<Consumer> &order_controller)
{
    const auto order_manager = order_controller.orderManager();
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
        
        order_controller.process(events::PlaceOrder<S, OrderType::Limit>
        {
            Ts { 1 },
            EventId { 1 },
            OrderId { 1 },
            Amount { 1 },
            Price { 100 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 1);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::PlaceOrder<S, OrderType::Limit>
        {
            Ts { 2 },
            EventId { 2 },
            OrderId { 2 },
            Amount { 2 },
            Price { 50 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 2);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::PlaceOrder<S, OrderType::Limit>
        {
            Ts { 3 },
            EventId { 3 },
            OrderId { 3 },
            Amount { 3 },
            Price { 150 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 3);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::PlaceOrder<S, OrderType::Limit>
        {
            Ts { 4 },
            EventId { 4 },
            OrderId { 4 },
            Amount { 4 },
            Price { 75 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 4);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::PlaceOrder<S, OrderType::Limit>
        {
            Ts { 5 },
            EventId { 5 },
            OrderId { 5 },
            Amount { 5 },
            Price { 125 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 5);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::PlaceOrder<S, OrderType::Limit>
        {
            Ts { 6 },
            EventId { 6 },
            OrderId { 6 },
            Amount { 6 },
            Price { 100 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 6);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::PlaceOrder<S, OrderType::Limit>
        {
            Ts { 7 },
            EventId { 7 },
            OrderId { 7 },
            Amount { 7 },
            Price { 50 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 7);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::PlaceOrder<S, OrderType::Limit>
        {
            Ts { 8 },
            EventId { 8 },
            OrderId { 8 },
            Amount { 8 },
            Price { 150 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::New);
            REQUIRE(event.client_order_id == 8);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        REQUIRE(limit_orders->size() == 8);
        
        order_controller.process(events::CancelOrder
        {
            Ts { 9 },
            EventId { 9 },
            OrderId { 1 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 1);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::CancelOrder
        {
            Ts { 10 },
            EventId { 10 },
            OrderId { 2 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 2);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::CancelOrder
        {
            Ts { 11 },
            EventId { 11 },
            OrderId { 3 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 3);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::CancelOrder
        {
            Ts { 12 },
            EventId { 12 },
            OrderId { 4 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 4);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::CancelOrder
        {
            Ts { 13 },
            EventId { 13 },
            OrderId { 5 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 5);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::CancelOrder
        {
            Ts { 14 },
            EventId { 14 },
            OrderId { 6 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 6);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::CancelOrder
        {
            Ts { 15 },
            EventId { 15 },
            OrderId { 7 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 7);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(is_sorted_by_price(limit_orders));
        
        order_controller.process(events::CancelOrder
        {
            Ts { 16 },
            EventId { 16 },
            OrderId { 8 }
        });
        
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            REQUIRE(OS == OrderStatus::Canceled);
            REQUIRE(event.client_order_id == 8);
        },
        consumer->orderUpdates().back());
        
        REQUIRE(limit_orders->empty());
    }
    
    SECTION("Fill order")
    {
        order_controller.process(events::PlaceOrder<S, OrderType::Limit>
        {
            Ts { 1 },
            EventId { 1 },
            OrderId { 1 },
            Amount { 100 },
            Price { 99 }
        });
    
        REQUIRE(limit_orders->front().filledAmount() == Amount { 0 });
        
        order_controller.process(events::FillOrder
        {
           Ts { 2 },
           EventId { 2 },
           OrderId { 1 },
           Amount { 10 }
        });
    
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
        consumer->orderUpdates().back());
    
        REQUIRE(limit_orders->front().filledAmount() == Amount { 10 });
    
        order_controller.process(events::FillOrder
        {
            Ts { 3 },
            EventId { 3 },
            OrderId { 1 },
            Amount { 20 }
        });
    
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
        consumer->orderUpdates().back());
    
        REQUIRE(limit_orders->front().filledAmount() == Amount { 30 });
    
        order_controller.process(events::FillOrder
        {
            Ts { 4 },
            EventId { 4 },
            OrderId { 1 },
            Amount { 30 }
        });
    
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
        consumer->orderUpdates().back());
    
        REQUIRE(limit_orders->front().filledAmount() == Amount { 60 });
    
        order_controller.process(events::FillOrder
        {
            Ts { 5 },
            EventId { 5 },
            OrderId { 1 },
            Amount { 40 }
        });
    
        std::visit([]<OrderStatus OS>(const events::OrderUpdate<OS> &event)
        {
            if constexpr (OS == OrderStatus::Filled) {
                REQUIRE(event.client_order_id == 1);
            }
            else {
                FAIL();
            }
        },
        consumer->orderUpdates().back());
    
        REQUIRE(limit_orders->empty());
    }
}

TEST_CASE("OrderController")
{
    auto consumer = std::make_shared<Consumer>();
    
    OrderController<Consumer> order_controller;
    order_controller.setConsumer(consumer);
    
    SIDE_SECTION(MarketOrders, consumer, order_controller);
    
    SIDE_SECTION(LimitOrders, consumer, order_controller);
}
