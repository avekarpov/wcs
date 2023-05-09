#ifndef WCS_DEBUG_HPP
#define WCS_DEBUG_HPP

#ifdef __GNUC__
    #ifndef NDEBUG
        #define WCS_DEBUG_BUILD
    #endif
#elif
    #error "Unknown compiler"
#endif

#endif //WCS_DEBUG_HPP
