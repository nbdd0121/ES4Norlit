#include "all.h"

#include "JSBoolean.h"
#include "JSString.h"
#include "object/JSObject.h"

#include <limits>
#include <algorithm>

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::object;

void Testing::RequireObjectCoercible(const Handle<JSValue>& val) {
    if (!val || val->GetType() == JSValue::Type::kNull) {
        Exceptions::ThrowTypeError("Cannot access property of null or undefined");
    }
}

bool Testing::IsCallable(const Handle<JSValue>& val) {
    if (val->GetType() != JSValue::Type::kObject) {
        return false;
    }
    return val.CastTo<JSObject>()->IsCallable();
}

bool Testing::IsConstructor(const Handle<JSValue>& val) {
    if (val->GetType() != JSValue::Type::kObject) {
        return false;
    }
    return val.CastTo<JSObject>()->IsConstructor();
}

bool Testing::SameValue(const Handle<JSValue>& x, const Handle<JSValue>& y) {
    if (x == y) return true;
    if (x->GetType() == JSValue::Type::kNumber&&y->GetType() == JSValue::Type::kNumber) {
        double xVal = x.CastTo<JSNumber>()->Value();
        double yVal = y.CastTo<JSNumber>()->Value();
        if (std::isnan(xVal)) return std::isnan(yVal);
        if (JSNumber::IsNegativeZero(xVal)) return JSNumber::IsNegativeZero(yVal);
        if (JSNumber::IsNegativeZero(yVal)) return false;
        return xVal == yVal;
    } else {
        return false;
    }
}

int Testing::IsSmaller(const Handle<JSPrimitive>& l, const Handle<JSPrimitive>& r) {
    JSValue::Type lType = l->GetType(), rType = r->GetType();
    if (lType == JSValue::Type::kString && rType == JSValue::Type::kString) {
        if (l == r) return false;

        Handle<JSString> lStr = l.CastTo<JSString>();
        Handle<JSString> rStr = r.CastTo<JSString>();

        size_t minLength = std::min(lStr->Length(), rStr->Length());
        for (size_t i = 0; i < minLength; i++) {
            int diff = lStr->At(i) - rStr->At(i);
            if (diff != 0) {
                if (diff < 0) {
                    return true;
                }
            }
        }
        return lStr->Length() < rStr->Length();
    } else {
        double nx = Conversion::ToNumber(l)->Value();
        double ny = Conversion::ToNumber(r)->Value();
        if (std::isnan(nx) || std::isnan(ny)) {
            return -1;
        }
        return nx < ny;
    }
}

bool Testing::IsAbstractlyEqual(const Handle<JSValue>& x, const Handle<JSValue>& y) {
    if (x == y)return true;
    JSValue::Type xType = x->GetType(), yType = y->GetType();
    if (xType == yType) {
        if (xType == JSValue::Type::kNumber) {
            return x.CastTo<JSNumber>()->Value() == y.CastTo<JSNumber>()->Value();
        } else {
            return false;
        }
    }

    // The only two cases that returns true when one operand is null or undefined
    if (xType == JSValue::Type::kNull) return yType == JSValue::Type::kUndefined;
    if (yType == JSValue::Type::kNull) return xType == JSValue::Type::kUndefined;
    if (xType == JSValue::Type::kUndefined) return false;
    if (yType == JSValue::Type::kUndefined) return false;

    if (xType == JSValue::Type::kNumber&&yType == JSValue::Type::kString) return IsAbstractlyEqual(x, Conversion::ToNumber(y));
    if (xType == JSValue::Type::kString&&yType == JSValue::Type::kNumber) return IsAbstractlyEqual(Conversion::ToNumber(x), y);
    if (xType == JSValue::Type::kBoolean) return IsAbstractlyEqual(Conversion::ToNumber(x), y);
    if (yType == JSValue::Type::kBoolean) return IsAbstractlyEqual(x, Conversion::ToNumber(y));

    if (xType == JSValue::Type::kObject) return IsAbstractlyEqual(Conversion::ToPrimitive(x), y);
    if (yType == JSValue::Type::kObject) return IsAbstractlyEqual(x, Conversion::ToPrimitive(y));

    return false;
}

bool Testing::IsStrictlyEqual(const Handle<JSValue>& x, const Handle<JSValue>& y) {
    if (x == y) return true;
    if (x->GetType() == JSValue::Type::kNumber&&y->GetType() == JSValue::Type::kNumber) {
        return x.CastTo<JSNumber>()->Value() == y.CastTo<JSNumber>()->Value();
    } else {
        return false;
    }
}