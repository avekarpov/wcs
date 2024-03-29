#include "../include/wcs/order_manager.hpp"

using namespace wcs;

UnsafeIndexedOrderList::UnsafeIndexedOrderList() : _list { std::make_shared<OrderList>() } { }

OrderHandler &UnsafeIndexedOrderList::get(wcs::OrderId id)
{
    return *_index.find(id)->second;
}

const OrderHandler &UnsafeIndexedOrderList::get(wcs::OrderId id) const
{
    return *_index.find(id)->second;
}

void UnsafeIndexedOrderList::remove(wcs::OrderId id)
{
    auto it = _index.find(id);
    
    _list->erase(it->second);
    _index.erase(it);
}

SharedOrderList UnsafeIndexedOrderList::list() const
{
    return _list;
}

OrderHandler &OrderManager::get(wcs::OrderId id)
{
    auto it = _order_table.find(id);
    
    if (it == _order_table.end()) {
        throw WCS_EXCEPTION(std::runtime_error, "Invalid order id");
    }
    
    switch (it->second)
    {
        case OrderVariant::MarketBuy: return _market_orders.get<Side::Buy>().get(id);
        case OrderVariant::MarketSell: return _market_orders.get<Side::Sell>().get(id);
        case OrderVariant::LimitBuy: return _limit_orders.get<Side::Buy>().get(id);
        case OrderVariant::LimitSell: return _limit_orders.get<Side::Sell>().get(id);
        default: throw WCS_EXCEPTION(std::logic_error, "Out of enum");
    }
}

const OrderHandler &OrderManager::get(wcs::OrderId id) const
{
    auto it = _order_table.find(id);
    
    if (it == _order_table.end()) {
        throw WCS_EXCEPTION(std::runtime_error, "Invalid order id");
    }
    
    switch (it->second)
    {
        case OrderVariant::MarketBuy: return _market_orders.get<Side::Buy>().get(id);
        case OrderVariant::MarketSell: return _market_orders.get<Side::Sell>().get(id);
        case OrderVariant::LimitBuy: return _limit_orders.get<Side::Buy>().get(id);
        case OrderVariant::LimitSell: return _limit_orders.get<Side::Sell>().get(id);
        default: throw WCS_EXCEPTION(std::logic_error, "Out of enum");
    }
}

void OrderManager::remove(wcs::OrderId id)
{
    auto it = _order_table.find(id);
    
    if (it == _order_table.end()) {
        throw WCS_EXCEPTION(std::runtime_error, "Invalid order id");
    }
    
    switch (it->second)
    {
        case OrderVariant::MarketBuy: _market_orders.get<Side::Buy>().remove(id); break;
        case OrderVariant::MarketSell: _market_orders.get<Side::Sell>().remove(id); break;
        case OrderVariant::LimitBuy: _limit_orders.get<Side::Buy>().remove(id); break;
        case OrderVariant::LimitSell: _limit_orders.get<Side::Sell>().remove(id); break;
        default: throw WCS_EXCEPTION(std::logic_error, "Out of enum");
    }
    
    _order_table.erase(id);
}

SidePair<wcs::OrderManager::SideSharedOrderList> OrderManager::marketOrders() const
{
    return { _market_orders.get<Side::Buy>().list(), _market_orders.get<Side::Sell>().list() };
}

SidePair<wcs::OrderManager::SideSharedOrderList> OrderManager::limitOrders() const
{
    return { _limit_orders.get<Side::Buy>().list(), _limit_orders.get<Side::Sell>().list() };
}
