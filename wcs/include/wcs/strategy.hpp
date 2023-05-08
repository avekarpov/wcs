#ifndef WCS_STRATEGY_HPP
#define WCS_STRATEGY_HPP

#include <memory>

#include "events/trade.hpp"
#include "events/order_book_update.hpp"
#include "events/order_update.hpp"
#include "virtual_exchange.hpp"

namespace wcs
{

class Exchange;

class Strategy
{
public:
    void setExchange(const std::shared_ptr<Exchange> &exchange)
    {
        _exchange = exchange;
    }

    virtual void on(const events::Trade &event) = 0;

    virtual void on(const events::OrderBookUpdate &event) = 0;

    virtual void on(const events::OrderUpdate<OrderStatus::Placed> &event) = 0;

    virtual void on(const events::OrderUpdate<OrderStatus::Partially> &event) = 0;

    virtual void on(const events::OrderUpdate<OrderStatus::Filled> &event) = 0;

    virtual void on(const events::OrderUpdate<OrderStatus::Canceled> &event) = 0;

    virtual void on(const events::OrderUpdate<OrderStatus::Rejected>&event) = 0;

    virtual ~Strategy() = default;

protected:
    std::weak_ptr<Exchange> _exchange;

};

} // namespace wcs

#endif //WCS_STRATEGY_HPP
