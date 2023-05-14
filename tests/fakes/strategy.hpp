#ifndef WCS_FAKES_STRATEGY_HPP
#define WCS_FAKES_STRATEGY_HPP

#include "wcs/strategy.hpp"

namespace wcs::fakes
{

class Strategy : public wcs::Strategy
{
public:
    void on(const events::Trade &event) override
    {

    }

    void on(const events::OrderBookUpdate &event) override
    {

    }

    void on(const events::OrderUpdate<OrderStatus::Placed> &event) override
    {

    }

    void on(const events::OrderUpdate<OrderStatus::Partially> &event) override
    {

    }

    void on(const events::OrderUpdate<OrderStatus::Filled> &event) override
    {

    }

    void on(const events::OrderUpdate<OrderStatus::Canceled> &event) override
    {

    }

    void on(const events::OrderUpdate<OrderStatus::Rejected> &event) override
    {

    }

};

} // namespace wcs::fakes

#endif //WCS_FAKES_STRATEGY_HPP
