#include <catch2/catch.hpp>

#include <wcs/order_controller.hpp>

#include "../spies/consumer.hpp"
#include "../utilits/side_section.hpp"

using namespace wcs;

template <Side S>
void MarketOrders(std::shared_ptr<Consumer> consumer, OrderController<Consumer> &order_controller)
{
    const auto market_orders = order_controller.marketOrders().get<S>();
    
    REQUIRE(market_orders);
    REQUIRE(market_orders->empty());
    
    order_controller.process(events::PlaceOrder<S, OrderType::Market>
    {
        Ts { 1 },
        EventId { 1 },
        OrderId { 1 },
        Amount<S> { 1 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
        REQUIRE(OS == OrderStatus::New);
        REQUIRE(event.client_order_id == 1);
    },
    consumer->orderUpdates().back());
    
    order_controller.process(events::PlaceOrder<S, OrderType::Market>
    {
        Ts { 2 },
        EventId { 2 },
        OrderId { 2 },
        Amount<S> { 2 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
        REQUIRE(OS == OrderStatus::New);
        REQUIRE(event.client_order_id == 2);
    },
    consumer->orderUpdates().back());
    
    order_controller.process(events::PlaceOrder<S, OrderType::Market>
    {
        Ts { 3 },
        EventId { 3 },
        OrderId { 3 },
        Amount<S> { 3 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
        REQUIRE(OS == OrderStatus::New);
        REQUIRE(event.client_order_id == 3);
    },
    consumer->orderUpdates().back());
    
    auto orders_it = market_orders->begin();
    REQUIRE(orders_it->id() == 1);
    REQUIRE(orders_it->amount() == Amount<S>{ 1 });
    
    ++orders_it;
    REQUIRE(orders_it->id() == 2);
    REQUIRE(orders_it->amount() == Amount<S>{ 2 });
    
    ++orders_it;
    REQUIRE(orders_it->id() == 3);
    REQUIRE(orders_it->amount() == Amount<S>{ 3 });
    
    REQUIRE(market_orders->size() == 3);
    
    order_controller.process(events::CancelOrder
    {
        Ts { 4 },
        EventId { 4 },
        OrderId{ 1 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
        REQUIRE(OS == OrderStatus::Canceled);
        REQUIRE(event.client_order_id == 1);
    },
    consumer->orderUpdates().back());
    
    orders_it = market_orders->begin();
    REQUIRE(orders_it->id() == 2);
    REQUIRE(orders_it->amount() == Amount<S>{ 2 });
    
    order_controller.process(events::CancelOrder
    {
        Ts { 5 },
        EventId { 5 },
        OrderId{ 2 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
        REQUIRE(OS == OrderStatus::Canceled);
        REQUIRE(event.client_order_id == 2);
    },
    consumer->orderUpdates().back());
    
    orders_it = market_orders->begin();
    REQUIRE(orders_it->id() == 3);
    REQUIRE(orders_it->amount() == Amount<S>{ 3 });
    
    order_controller.process(events::CancelOrder
    {
        Ts { 6 },
        EventId { 6 },
        OrderId{ 3 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
        REQUIRE(OS == OrderStatus::Canceled);
        REQUIRE(event.client_order_id == 3);
    },
    consumer->orderUpdates().back());
    
    REQUIRE(market_orders->empty());
}

template <Side S>
void LimitOrders(std::shared_ptr<Consumer> consumer, OrderController<Consumer> &order_controller)
{
    const auto is_sorted_by_price = [](const auto &orders)
    {
        Price<S> previous = orders->begin()->price();
        for (auto it = ++orders->begin(); it != orders->end(); ++it)
        {
            if (utilits::sideLess(previous, it->price())) {
                return false;
            }

            previous = it->price();
        }

        return true;
    };
    
    const auto limit_orders = order_controller.limitOrders().get<S>();
    
    REQUIRE(limit_orders);
    REQUIRE(limit_orders->empty());
    
    order_controller.process(events::PlaceOrder<S, OrderType::Limit>
    {
        Ts { 1 },
        EventId { 1 },
        OrderId { 1 },
        Amount<S> { 1 },
        Price<S> { 100 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
        Amount<S> { 2 },
        Price<S> { 50 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
        Amount<S> { 3 },
        Price<S> { 150 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
        Amount<S> { 4 },
        Price<S> { 75 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
        Amount<S> { 5 },
        Price<S> { 125 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
        Amount<S> { 6 },
        Price<S> { 100 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
        Amount<S> { 7 },
        Price<S> { 50 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
        Amount<S> { 8 },
        Price<S> { 150 }
    });
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
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
    
    std::visit([]<Side A, OrderStatus OS>(const events::OrderUpdate<A, OS> &event)
    {
        REQUIRE(A == S);
        REQUIRE(OS == OrderStatus::Canceled);
        REQUIRE(event.client_order_id == 8);
    },
    consumer->orderUpdates().back());
    
    REQUIRE(limit_orders->empty());
}

TEST_CASE("OrderController")
{
    auto consumer = std::make_shared<Consumer>();
    
    OrderController<Consumer> order_controller;
    order_controller.setConsumer(consumer);
    
    SIDE_SECTION(MarketOrders, consumer, order_controller);
    
    SIDE_SECTION(LimitOrders, consumer, order_controller);
}
