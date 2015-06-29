#include "Builtin.h"

#include "../JSBoolean.h"
#include "../Conversion.h"
#include "../Objects.h"
#include "../Exception.h"

#include "../object/Exotics.h"
#include "../vm/Realm.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::builtin;

namespace {
static Handle<JSBoolean> thisBooleanValue(const Handle<JSValue>& val, const char* caller) {
    switch (val->GetType()) {
        case JSValue::Type::kBoolean:
            return val.CastTo<JSBoolean>();
        case JSValue::Type::kObject:
            if (Handle<BooleanObject> b = val.ExactCheckedCastTo<BooleanObject>()) {
                return JSBoolean::New(b->booleanData());
            }
        default:
            Exceptions::ThrowIncompatibleReceiverTypeError(caller);
    }
}
}

Handle<JSValue> Boolean::Call(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    return Conversion::ToBoolean(GetArg(args, 0));
}

Handle<JSObject> Boolean::Construct(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {
    bool b = Conversion::ToBooleanValue(GetArg(args, 0));
    Handle<BooleanObject>  O = Objects::OrdinaryCreateFromConstructor<BooleanObject>(target, &Realm::BooleanPrototype);
    O->booleanData(b);
    return O;
}

Handle<JSValue> Boolean::prototype::toString(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    return Conversion::ToString(thisBooleanValue(that, "Boolean.prototype.toString"));
}

Handle<JSValue> Boolean::prototype::valueOf(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    return thisBooleanValue(that, "Boolean.prototype.valueOf");
}

