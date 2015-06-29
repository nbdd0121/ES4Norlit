#include "JSValue.h"

using namespace norlit::js;
using namespace norlit::gc;

JSValue::Type JSValue::GetType() const {
    if (IsHeapObject()) {
        if (!this) {
            return Type::kUndefined;
        } else {
            return VirtualGetType();
        }
    } else if (IsSmallInteger()) {
        return Type::kNumber;
    } else if (IsShortString()) {
        return Type::kString;
    } else if (IsBoolean()) {
        return Type::kBoolean;
    } else {
        assert(IsNull());
        return Type::kNull;
    }
}
