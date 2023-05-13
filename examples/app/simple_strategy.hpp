#ifndef WCS_SIMPLE_STRATEGY_HPP
#define WCS_SIMPLE_STRATEGY_HPP

#include <list>

#include <wcs/strategy.hpp>

class SimpleStrategy : public wcs::Strategy
{
private:
    using Ticker = wcs::SidePair<wcs::Level>;

    struct MarketOrderState
    {
        wcs::Side side;
        wcs::OrderId id;
        wcs::Amount amount;

        wcs::OrderStatus status;

        // wcs::Price wa_price;
        wcs::Amount filled_amount;

        bool send;
    };

public:
    SimpleStrategy(const wcs::Time &price_change_window, const double price_changed_coef)
    :
        _price_changed_window { price_change_window },
        _price_changed_coef { price_changed_coef },
        _next_order_id { 1 },
        _order_state { .send = false },
        _position { 0 }
    {

    }

    void on(const wcs::events::Trade &event) override
    {
        // skip
    }

    void on(const wcs::events::OrderBookUpdate &event) override
    {
        _logger.gotEvent(event);

        const auto ticker = createTicker(event);
        const auto middle_price = middlePrice(ticker);
        _middle_prices_by_ts.emplace_front(middle_price, event.ts);

        if (_middle_prices_by_ts.size() > 2) {
            while (_middle_prices_by_ts.back().second < _middle_prices_by_ts.front().second - _price_changed_window) {
                _middle_prices_by_ts.pop_back();
            }

            double price_changed { 0 };
            auto it = _middle_prices_by_ts.cbegin();
            auto prev_it = it++;
            while (it != _middle_prices_by_ts.end()) {
                price_changed += priceDif(it->first, prev_it->first);;

                prev_it = it++;
            }


            if (!_order_state.send) {
               _logger.debug(R"(Price change: {}, middle price {})", price_changed, middle_price);

               if (std::fabs(price_changed) / static_cast<double>(middle_price) >= _price_changed_coef) {
                   if (price_changed > 0) {
                       _logger.info(R"(Place buy order)");

                       placeMarketOrder<wcs::Side::Buy>(ticker.get<wcs::Side::Buy>().volume() / 100);
                   } else {
                       _logger.info(R"(Place sell order)");

                       placeMarketOrder<wcs::Side::Sell>(ticker.get<wcs::Side::Sell>().volume() / 100);
                   }
               }
            }
        }
    }

    void on(const wcs::events::OrderUpdate<wcs::OrderStatus::Placed> &event) override
    {
        _logger.gotEvent(event);

        assert(_order_state.id == event.client_order_id);

        _order_state.status = wcs::OrderStatus::Placed;
    }

    void on(const wcs::events::OrderUpdate<wcs::OrderStatus::Partially> &event) override
    {
        _logger.gotEvent(event);

        assert(_order_state.id == event.client_order_id);

        _order_state.status = wcs::OrderStatus::Partially;

        _order_state.filled_amount = event.amount;
    }

    void on(const wcs::events::OrderUpdate<wcs::OrderStatus::Filled> &event) override
    {
        _logger.gotEvent(event);

        assert(_order_state.id == event.client_order_id);

        _order_state.status = wcs::OrderStatus::Canceled;

        _order_state.filled_amount = event.amount;
        _position += _order_state.side == wcs::Side::Buy
            ? _order_state.filled_amount
            : _order_state.filled_amount * -1.0;

        _logger.info("Order: {} filled", _order_state.filled_amount);
        _logger.info("Position: {}", _position);

        _order_state.send = false;
    }

    void on(const wcs::events::OrderUpdate<wcs::OrderStatus::Canceled> &event) override
    {
        _logger.gotEvent(event);

        assert(_order_state.id == event.client_order_id);

        _order_state.status = wcs::OrderStatus::Canceled;

        _order_state.send = false;
    }

    void on(const wcs::events::OrderUpdate<wcs::OrderStatus::Rejected> &event) override
    {
        _logger.gotEvent(event);

        assert(_order_state.id == event.client_order_id);

        _order_state.status = wcs::OrderStatus::Rejected;

        _order_state.send = false;
    }

private:
    template <wcs::Side S>
    void placeMarketOrder(const wcs::Amount &amount)
    {
        assert(!wcs::isExecution(_order_state.status));

        _order_state.side = S;
        _order_state.id = _next_order_id++;
        _order_state.amount = amount;

        _order_state.status = wcs::OrderStatus::New;
        // _order_state.wa_price = wcs::Price { 0 };
        _order_state.filled_amount = wcs::Amount { 0 };

        _exchange.lock()->placeMarketOrder(_order_state.id, _order_state.side, _order_state.amount);

        _order_state.send = true;
    }

    static Ticker createTicker(const wcs::events::OrderBookUpdate &event)
    {
        return Ticker
        {
            {
                event.depth.get<wcs::Side::Buy>().front().price(),
                event.depth.get<wcs::Side::Buy>().front().volume()
            },
            {
                event.depth.get<wcs::Side::Sell>().front().price(),
                event.depth.get<wcs::Side::Sell>().front().volume()
            }
        };
    }

    static wcs::Price middlePrice(const Ticker &ticker)
    {
        return (ticker.get<wcs::Side::Buy>().price() + ticker.get<wcs::Side::Sell>().price()) / 2.0;
    }

    static double priceDif(const wcs::Price &lhs, const wcs::Price &rhs)
    {
        return static_cast<double>(lhs) - static_cast<double>(rhs);
    }

private:
    const wcs::Time _price_changed_window;
    const double _price_changed_coef;

    std::list<std::pair<wcs::Price, wcs::Ts>> _middle_prices_by_ts;

    wcs::OrderId _next_order_id;
    MarketOrderState _order_state;
    wcs::Amount _position;

    wcs::Logger _logger { "SimpleStrategy" };
};

#endif //WCS_SIMPLE_STRATEGY_HPP
