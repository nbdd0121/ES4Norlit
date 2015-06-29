#ifndef NORLIT_JS_JSNUMBER_H
#define NORLIT_JS_JSNUMBER_H

#include "JSPrimitive.h"
#include "../gc/Handle.h"
#include "../meta/Math.h"

#include <cmath>
#include <limits>

namespace norlit {
namespace js {

class JSNumber final : public JSPrimitive {
  private:
    static const size_t  MAX_SMI_BITS = meta::Math<size_t>::Min<54, sizeof(void*) * 8 - 1>::value;
    static const int64_t MAX_SMI_VALUE = (static_cast<int64_t>(1) << (MAX_SMI_BITS - 1)) - 1;
    static const int64_t MIN_SMI_VALUE = -(static_cast<int64_t>(1) << (MAX_SMI_BITS - 1));

    static JSNumber* CreateSmallInteger(int64_t);
    static JSNumber* CreateInstance(double);

    double value;

    JSNumber(double);
    int64_t GetSmallIntegerValue() const;
    virtual Type VirtualGetType() const override;

  public:
    static gc::Handle<JSNumber> New(int64_t);
    static gc::Handle<JSNumber> New(int32_t);
    static gc::Handle<JSNumber> New(double);

    static bool IsNegativeZero(double);

    static gc::Handle<JSNumber> NaN();
    static gc::Handle<JSNumber> Infinity();
    static gc::Handle<JSNumber> Zero();
    static gc::Handle<JSNumber> One();

    gc::Handle<JSNumber> Negate();

    double Value() const;
};

inline JSNumber* JSNumber::CreateSmallInteger(int64_t value) {
    return reinterpret_cast<JSNumber*>((static_cast<intptr_t>(value) << 1) | 1);
}

inline JSNumber* JSNumber::CreateInstance(double value) {
    return new JSNumber(value);
}

inline bool JSNumber::IsNegativeZero(double number) {
    return number == 0 && std::signbit(number);
}

inline int64_t JSNumber::GetSmallIntegerValue() const {
    return reinterpret_cast<intptr_t>(this) >> 1;
}

inline gc::Handle<JSNumber> JSNumber::New(int64_t value) {
    if (value > MAX_SMI_VALUE || value < MIN_SMI_VALUE) {
        return CreateInstance(static_cast<double>(value));
    } else {
        return CreateSmallInteger(value);
    }
}

inline gc::Handle<JSNumber> JSNumber::New(int32_t value) {
    return New(static_cast<int64_t>(value));
}

inline gc::Handle<JSNumber> JSNumber::New(double value) {
    int64_t intValue = (int64_t)value;
    if (intValue == value && !IsNegativeZero(value)) {
        return New(intValue);
    } else {
        return CreateInstance(value);
    }
}

inline double JSNumber::Value() const {
    if (IsSmallInteger()) {
        return static_cast<double>(GetSmallIntegerValue());
    }
    return value;
}

inline gc::Handle<JSNumber> JSNumber::NaN() {
    return New(std::numeric_limits<double>::quiet_NaN());
}

inline gc::Handle<JSNumber> JSNumber::Infinity() {
    return New(std::numeric_limits<double>::infinity());
}

inline gc::Handle<JSNumber> JSNumber::Zero() {
    return CreateSmallInteger(0);
}

inline gc::Handle<JSNumber> JSNumber::One() {
    return CreateSmallInteger(1);
}

inline gc::Handle<JSNumber> JSNumber::Negate() {
    return New(-Value());
}

}
}

#endif
