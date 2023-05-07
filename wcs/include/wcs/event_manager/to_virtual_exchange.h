#ifndef WCS_TO_VIRTUAL_EXCHANGE_H
#define WCS_TO_VIRTUAL_EXCHANGE_H

#include <memory>

#include "../events/trade.hpp"
#include "../events/order_book_update.hpp"
#include "../events/order_update.hpp"

namespace wcs
{

template <template <class> class VirtualExchange_t, class EventManager_t>
class ToVirtualExchangeBase
{
public:
    void setVirtualExchange(const std::shared_ptr<VirtualExchange_t<EventManager_t>> &virtual_exchange)
    {
        _virtual_exchange = virtual_exchange;
    }

    void process(const events::Trade &event)
    {
        _virtual_exchange.lock()->process(event);
    }

    void process(const events::OrderBookUpdate &event)
    {
        _virtual_exchange.lock()->process(event);
    }

    template<OrderStatus OS>
    void process(const events::OrderUpdate<OS> &event)
    {
        _virtual_exchange.lock()->process(event);
    }

private:
    std::weak_ptr<VirtualExchange_t<EventManager_t>> _virtual_exchange;

};

} // namespace wcs

#endif //WCS_TO_VIRTUAL_EXCHANGE_H
