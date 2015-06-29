#ifndef NORLIT_UTIL_SCOPEEXIT_H
#define NORLIT_UTIL_SCOPEEXIT_H

#include "Preprocessing.h"

namespace norlit {
namespace util {

template<typename F>
struct ScopeExitImpl {
    F func;
    ScopeExitImpl(F f):func(f) {}
    ScopeExitImpl(const ScopeExitImpl&) = delete;
    ~ScopeExitImpl() {
        func();
    }
};

struct ScopeExitBuilder {
    template<typename F>
    ScopeExitImpl<F> operator <<(F f) {
        return{ f };
    }
};

}
}

#ifdef _MSC_VER
#define NORLIT_SCOPE_EXIT auto&& NORLIT_PP_CONCAT(norlit_scope_exit_, __LINE__) = norlit::util::ScopeExitBuilder{} << [&]()
#else
#define NORLIT_SCOPE_EXIT __attribute__((unused)) auto&& NORLIT_PP_CONCAT(norlit_scope_exit_, __LINE__) = norlit::util::ScopeExitBuilder{} << [&]()
#endif

#endif