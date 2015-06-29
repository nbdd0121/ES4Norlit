#ifndef NORLIT_UTIL_STATIC_BLOCK_H
#define NORLIT_UTIL_STATIC_BLOCK_H

#include "Preprocessing.h"

namespace norlit {
namespace util {
struct StaticInitializerImpl {
    template<typename T>
    bool operator <<(const T& t) {
        t();
        return true;
    }
};
}
}

#ifdef _MSC_VER
#define NORLIT_STATIC_BLOCK static bool NORLIT_PP_CONCAT(norlit_static_init_, __LINE__) = norlit::util::StaticInitializerImpl{} << [&]()
#else
#define NORLIT_STATIC_BLOCK static __attribute__((unused)) bool NORLIT_PP_CONCAT(norlit_static_init_, __LINE__) = norlit::util::StaticInitializerImpl{} << [&]()
#endif

#endif