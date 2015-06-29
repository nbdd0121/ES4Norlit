#ifndef NORLIT_JS_TESTING_H
#define NORLIT_JS_TESTING_H

#include "JSValue.h"

namespace norlit {
namespace js {

class JSPrimitive;
class JSString;
class JSPropertyKey;
class JSNumber;
class JSBoolean;
class JSSymbol;

namespace object {
class JSObject;
}

namespace detail {
template<typename T>
struct TypeChecker {};

template<>
struct TypeChecker < JSNull > {
    static bool Is(const gc::Handle<JSValue>& op) {
        return op->GetType() == JSValue::Type::kNull;
    }
};

template<>
struct TypeChecker < JSString > {
    static bool Is(const gc::Handle<JSValue>& op) {
        return op->GetType() == JSValue::Type::kString;
    }
};

template<>
struct TypeChecker < JSNumber > {
    static bool Is(const gc::Handle<JSValue>& op) {
        return op->GetType() == JSValue::Type::kNumber;
    }
};

template<>
struct TypeChecker < JSBoolean > {
    static bool Is(const gc::Handle<JSValue>& op) {
        return op->GetType() == JSValue::Type::kBoolean;
    }
};

template<>
struct TypeChecker < JSSymbol > {
    static bool Is(const gc::Handle<JSValue>& op) {
        return op->GetType() == JSValue::Type::kSymbol;
    }
};

template<>
struct TypeChecker < JSPropertyKey > {
    static bool Is(const gc::Handle<JSValue>& op) {
        switch (op->GetType()) {
            case JSValue::Type::kString:
            case JSValue::Type::kSymbol:
                return true;
            default:
                return false;
        }
    }
};

template<>
struct TypeChecker < JSPrimitive > {
    static bool Is(const gc::Handle<JSValue>& op) {
        return op->GetType() != JSValue::Type::kObject;
    }
};

template<>
struct TypeChecker < object::JSObject > {
    static bool Is(const gc::Handle<JSValue>& op) {
        return op->GetType() == JSValue::Type::kObject;
    }
};


}

class Testing {
  public:
    template<typename T>
    static bool Is(const gc::Handle<JSValue>& op) {
        return detail::TypeChecker<T>::Is(op);
    }

    template<typename T>
    static gc::Handle<T> CastIf(const gc::Handle<JSValue>& op) {
        if (Is<T>(op)) {
            return op.CastTo<T>();
        } else {
            return nullptr;
        }
    }

    static void RequireObjectCoercible(const gc::Handle<JSValue>&);

    static bool IsCallable(const gc::Handle<JSValue>&);
    static bool IsConstructor(const gc::Handle<JSValue>&);

    static bool SameValue(const gc::Handle<JSValue>&, const gc::Handle<JSValue>&);
    //  1 true
    //  0 false
    // -1 undefined
    static int IsSmaller(const gc::Handle<JSPrimitive>&, const gc::Handle<JSPrimitive>&);
    static bool IsAbstractlyEqual(const gc::Handle<JSValue>&, const gc::Handle<JSValue>&);
    static bool IsStrictlyEqual(const gc::Handle<JSValue>&, const gc::Handle<JSValue>&);
};

}
}

#endif
