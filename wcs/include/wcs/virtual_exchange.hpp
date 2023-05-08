#ifndef WCS_VIRTUAL_EXCHANGE_HPP
#define WCS_VIRTUAL_EXCHANGE_HPP

#include <memory>
#include <map>
#include <queue>
#include <variant>

#include "events/cancel_order.hpp"
#include "events/order_book_update.hpp"
#include "events/order_update.hpp"
#include "events/place_order.hpp"
#include "events/trade.hpp"
#include "logger.hpp"
#include "event_builder.hpp"
#include "time_manager.hpp"
#include "strategy.hpp"

namespace wcs
{

class Strategy;

class Exchange
{
public:
    virtual void placeLimitOrder(Side side, const OrderId &id, const Price &price, const Amount &amount) = 0;

    virtual void placeMarketOrder(Side side, const OrderId &id, const Amount &amount) = 0;

    virtual void cancelOrder(const OrderId &id) = 0;

    virtual ~Exchange() = default;
};

class VirtualExchangeLogger
{
protected:
    inline static Logger _logger { "VirtualExchange" };
};

// TODO: add test
template <class EventManager_t>
class VirtualExchange : public VirtualExchangeLogger, public Exchange
{
private:
    using BacktestEngine = typename EventManager_t::ToBacktestEngine;

public:
    void setEventManager(const std::shared_ptr<EventManager_t> &event_manager)
    {
        _backtest_engine = event_manager;
    }

    void setStrategy(const std::shared_ptr<Strategy> &strategy)
    {
        _strategy = strategy;
    }

    void placeLimitOrder(Side side, const OrderId &id, const Price &price, const Amount &amount) override
    {
        if (side == Side::Buy) {
            generatePlaceOrder<Side::Buy, OrderType::Limit>(id, price, amount);
        }
        else {
            generatePlaceOrder<Side::Sell, OrderType::Limit>(id, price, amount);
        }
    }

    void placeMarketOrder(Side side, const OrderId &id, const Amount &amount) override
    {
        if (side == Side::Buy) {
            generatePlaceOrder<Side::Buy, OrderType::Market>(id, amount);
        }
        else {
            generatePlaceOrder<Side::Sell, OrderType::Market>(id, amount);
        }
    };

    void cancelOrder(const OrderId &id) override
    {
        generateCancelOrder(id);
    }

    void process(const events::Trade &event)
    {
        _strategy.lock()->on(event);
    }

    void process(const events::OrderBookUpdate &event)
    {
        _strategy.lock()->on(event);
    }

    template <OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
    {
        static_assert(OS != OrderStatus::New);

        _strategy.lock()->on(event);
    }

private:
    template <Side S, OrderType OT, class ... Args>
    void generatePlaceOrder(Args &&...args)
    {
        _backtest_engine.lock()->process(EventBuilder::build<events::PlaceOrder<S, OT>>(
            TimeManager::time(),
            std::forward<Args>(args)...
        ));
    }

    void  generateCancelOrder(const OrderId &id)
    {
        _backtest_engine.lock()->process(EventBuilder::build<events::CancelOrder>(
            TimeManager::time(),
            id
        ));
    }

private:
    std::weak_ptr<Strategy> _strategy;
    std::weak_ptr<BacktestEngine> _backtest_engine;

};

} // namespace wcs

#endif //WCS_VIRTUAL_EXCHANGE_HPP
