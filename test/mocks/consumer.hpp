#ifndef WCS_CONSUMER_HPP
#define WCS_CONSUMER_HPP

namespace wcs::mocks
{

class Consumer
{
public:
    template <class Event>
    void process (const Event &event)
    {
    
    }
    
};

} // namespace wcs::mocks

#endif //WCS_CONSUMER_HPP
