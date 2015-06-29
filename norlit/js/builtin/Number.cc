#include "Builtin.h"

#include "../JSNumber.h"
#include "../JSBoolean.h"
#include "../Conversion.h"
#include "../Objects.h"
#include "../Testing.h"
#include "../Exception.h"

#include "../object/Exotics.h"
#include "../vm/Realm.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::builtin;

namespace {
static Handle<JSNumber> thisNumberValue(const Handle<JSValue>& val, const char* caller) {
    switch (val->GetType()) {
        case JSValue::Type::kNumber:
            return val.CastTo<JSNumber>();
        case JSValue::Type::kObject:
            if (Handle<NumberObject> n = val.ExactCheckedCastTo<NumberObject>()) {
                return JSNumber::New(n->numberData());
            }
        default:
            Exceptions::ThrowIncompatibleReceiverTypeError(caller);
    }
}
}

Handle<JSValue> Number::Call(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    if (args->Length() == 0) {
        return JSNumber::Zero();
    } else {
        return Conversion::ToNumber(GetArg(args, 0));
    }
}

Handle<JSObject> Number::Construct(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {
    double n;
    if (args->Length() == 0) {
        n = 0;
    } else {
        n = Conversion::ToNumberValue(GetArg(args, 0));
    }
    Handle<NumberObject> O = Objects::OrdinaryCreateFromConstructor<NumberObject>(target, &Realm::NumberPrototype);
    O->numberData(n);
    return O;
}

Handle<JSValue> Number::isFinite(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSNumber> number = Testing::CastIf<JSNumber>(GetArg(args, 0));
    if (!number) {
        return JSBoolean::New(false);
    }
    return JSBoolean::New(std::isfinite(number->Value()));
}

Handle<JSValue> Number::isInteger(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSNumber> number = Testing::CastIf<JSNumber>(GetArg(args, 0));
    if (!number) {
        return JSBoolean::New(false);
    }
    if (!std::isfinite(number->Value())) {
        return JSBoolean::New(false);
    }
    int64_t integer = Conversion::ToIntegerValue(number);
    return JSBoolean::New(integer == number->Value());
}

Handle<JSValue> Number::isNaN(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSNumber> number = Testing::CastIf<JSNumber>(GetArg(args, 0));
    if (!number) {
        return JSBoolean::New(false);
    }
    return JSBoolean::New(std::isnan(number->Value()));
}

Handle<JSValue> Number::isSafeInteger(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSNumber> number = Testing::CastIf<JSNumber>(GetArg(args, 0));
    if (!number) {
        return JSBoolean::New(false);
    }
    if (!std::isfinite(number->Value())) {
        return JSBoolean::New(false);
    }
    int64_t integer = Conversion::ToIntegerValue(number);
    if (integer != number->Value()) {
        return JSBoolean::New(false);
    }
    return JSBoolean::New(integer > -9007199254740992LL && integer < 9007199254740992LL);
}

Handle<JSValue> Number::prototype::valueOf(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    return thisNumberValue(that, "Number.prototype.valueOf");
}


