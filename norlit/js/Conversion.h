#ifndef NORLIT_JS_CONVERSION_H
#define NORLIT_JS_CONVERSION_H

#include "JSNumber.h"

namespace norlit {
namespace js {

class JSString;
class JSBoolean;
class JSPropertyKey;

namespace object {
class JSObject;
}

class Conversion {
  public:
    static gc::Handle<JSPrimitive> ToPrimitive(
        const gc::Handle<JSValue>&,
        JSValue::Type preferredType = JSValue::Type::kUndefined
    );
    static gc::Handle<JSPrimitive> OrdinaryToPrimitive(const gc::Handle<object::JSObject>&, JSValue::Type preferredType);
    static gc::Handle<JSBoolean> ToBoolean(const gc::Handle<JSValue>&);
    static bool ToBooleanValue(const gc::Handle<JSValue>&);
    static gc::Handle<JSNumber> ToNumber(const gc::Handle<JSValue>&);
    static double ToNumberValue(const gc::Handle<JSValue>&);
    static gc::Handle<JSString> ToString(const gc::Handle<JSValue>&);
    static gc::Handle<object::JSObject> ToObject(const gc::Handle<JSValue>&);
    static gc::Handle<JSPropertyKey> ToPropertyKey(const gc::Handle<JSValue>&);

    static int64_t ToIntegerValue(const gc::Handle<JSNumber>&);
    static int32_t ToInt32(const gc::Handle<JSNumber>&);
    static uint32_t ToUInt32(const gc::Handle<JSNumber>&);

    static int64_t ToIntegerIndex(const gc::Handle<JSString>&);
    static int64_t ToIntegerIndex(const gc::Handle<JSPropertyKey>&);

    static int64_t ToLength(const gc::Handle<JSNumber>&);
    static int64_t ToLength(const gc::Handle<JSValue>&);

    static gc::Handle<JSNumber> ScanInt(const gc::Handle<JSString>&, size_t&, uint8_t);
    static gc::Handle<JSNumber> ScanFloat(const gc::Handle<JSString>&, size_t&);
};

}
}

#endif
