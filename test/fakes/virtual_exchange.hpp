#ifndef WCS_VIRTUAL_EXCHANGE_HPP
#define WCS_VIRTUAL_EXCHANGE_HPP

namespace wcs::fakes
{

template <class EventManager_t>
class VirtualExchange
{
public:
    template <class Event_t>
    void process(const Event_t &)
    {

    }

};

} // namespace wcs::fakes

#endif //WCS_VIRTUAL_EXCHANGE_HPP
