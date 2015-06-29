#ifndef NORLIT_JS_JSVALUE_H
#define NORLIT_JS_JSVALUE_H

#include "../gc/Object.h"
#include "../gc/Handle.h"

namespace norlit {
namespace js {

class JSValue : public gc::Object {
  public:
    enum class Type {
        kUndefined,
        kNull,
        kBoolean,
        kNumber,
        kString,
        kSymbol,
        kObject
    };
  protected:
    bool IsSmallInteger() const;
    bool IsShortString() const;
    bool IsBoolean() const;
    bool IsNull() const;
    bool IsHeapObject() const;

    // Use of virtual on tagged objects are prohibited
    virtual Type VirtualGetType() const = 0;
  public:
    Type GetType() const;
};

inline bool JSValue::IsSmallInteger() const {
    return (reinterpret_cast<uintptr_t>(this) & 1) == 1;
}

inline bool JSValue::IsShortString() const {
    return (reinterpret_cast<uintptr_t>(this) & 7) == 2;
}

inline bool JSValue::IsBoolean() const {
    return (reinterpret_cast<uintptr_t>(this) & 7) == 4;
}

inline bool JSValue::IsNull() const {
    return (reinterpret_cast<uintptr_t>(this) & 7) == 6;
}

inline bool JSValue::IsHeapObject() const {
    return !IsTagged();
}

}
}

#endif
