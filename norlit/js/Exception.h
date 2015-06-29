#ifndef NORLIT_JS_ESEXCEPTION_H
#define NORLIT_JS_ESEXCEPTION_H

#include "JSValue.h"

#ifdef _MSC_VER
#define NORLIT_NORETURN __declspec(noreturn)
#else
#define NORLIT_NORETURN __attribute__((noreturn))
#endif

namespace norlit {
namespace js {

class JSString;

class ESException {
    gc::Handle<JSValue> val;

  public:
    ESException(const gc::Handle<JSValue>& val) :val(val) {}

    gc::Handle<JSValue> value() const {
        return val;
    }
};

struct Exceptions {
    NORLIT_NORETURN static void ThrowIncompatibleReceiverTypeError(const char* name);
    NORLIT_NORETURN static void ThrowTypeError(const char* name);
    NORLIT_NORETURN static void ThrowSyntaxError(const char* name);
    NORLIT_NORETURN static void ThrowRangeError(const char* name);
    NORLIT_NORETURN static void ThrowReferenceError(const gc::Handle<JSString>&);
};

}
}

#endif