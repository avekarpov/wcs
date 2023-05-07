#ifndef WCS_EVENT_MANAGER_HPP
#define WCS_EVENT_MANAGER_HPP

#include "../events/events.hpp"

namespace wcs
{

template <template <class> class ToVirtualExchange_t, template <class> class ToBacktestEngine_t>
class EventManagerBase :
    public ToVirtualExchange_t<EventManagerBase<ToVirtualExchange_t, ToBacktestEngine_t>>,
    public ToBacktestEngine_t<EventManagerBase<ToVirtualExchange_t, ToBacktestEngine_t>>
{
private:
    using ThisClass = EventManagerBase<ToVirtualExchange_t, ToBacktestEngine_t>;

public:
    using ToVirtualExchange = ToVirtualExchange_t<ThisClass>;
    using ToBacktestEngine = ToBacktestEngine_t<ThisClass>;

private:
    using ToVirtualExchange::process;
    using ToBacktestEngine::process;

};

} // namespace wcs

#endif //WCS_EVENT_MANAGER_HPP
