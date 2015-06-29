#ifndef NORLIT_META_MATH_H
#define NORLIT_META_MATH_H

namespace norlit {
namespace meta {

template<typename T>
struct Math {
    template<T a, T... b>
    struct Min {
        static const T value = a > Min<b...>::value ? Min<b...>::value : a;
    };

    template<T a>
    struct Min<a> {
        static const T value = a;
    };

};

}
}

#endif