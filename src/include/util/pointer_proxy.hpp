#ifndef UTIL_POINTER_PROXY_HPP
#define UTIL_POINTER_PROXY_HPP
#include <type_traits>

namespace util {
    
    template<class Ref>
    struct pointer_proxy {
        Ref ref;
        template<class ...Args>
        pointer_proxy(Args&& ...args) : ref(std::forward<Args>(args)...) {}

        Ref* operator->() { return &ref; }
    };

}

#endif