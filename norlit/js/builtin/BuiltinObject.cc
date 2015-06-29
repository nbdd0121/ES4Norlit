#include "Builtin.h"

#include "../object/JSOrdinaryObject.h"
#include "../vm/Context.h"
#include "../vm/Realm.h"

#include "../Conversion.h"
#include "../Objects.h"
#include "../Testing.h"
#include "../Exception.h"

#include "../JSString.h"
#include "../JSSymbol.h"
#include "../JSBoolean.h"

#include "../object/Exotics.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::builtin;
using namespace norlit::util;

Handle<JSValue> BuiltinObject::Call(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSValue> value = GetArg(args, 0);
    if (!value || value->GetType() == JSValue::Type::kNull) {
        Handle<JSObject> proto = Context::CurrentRealm()->ObjectPrototype();
        return Objects::ObjectCreate(proto);
    } else {
        return Conversion::ToObject(value);
    }
}

Handle<JSObject> BuiltinObject::Construct(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {
    if (target != Context::CurrentContext()->function()) {
        return Objects::OrdinaryCreateFromConstructor<JSOrdinaryObject>(target, &Realm::ObjectPrototype);
    }
    Handle<JSValue> value = GetArg(args, 0);
    if (!value || value->GetType() == JSValue::Type::kNull) {
        Handle<JSObject> proto = Context::CurrentRealm()->ObjectPrototype();
        return Objects::ObjectCreate(proto);
    } else {
        return Conversion::ToObject(value);
    }
}

//Handle<JSValue> BuiltinObject::assign(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
//}

Handle<JSValue> BuiltinObject::create(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSValue> O = GetArg(args, 0);
    if (O->GetType() == JSValue::Type::kNull) {
        O = nullptr;
    } else if (!Testing::Is<JSObject>(O)) {
        Exceptions::ThrowTypeError("Object prototype may only be object or null");
    }
    Handle<JSObject> obj = Objects::ObjectCreate(O.CastTo<JSObject>());
    if (Handle<JSValue> Properties = GetArg(args, 1)) {
        throw "TODO: Object.create";
        // Return ObjectDefineProperties(obj, Properties).
    }
    return obj;
}

//Handle<JSValue> BuiltinObject::defineProperties(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
//}

Handle<JSValue> BuiltinObject::defineProperty(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSObject> O = Testing::CastIf<JSObject>(GetArg(args, 0));
    if (!O) {
        Exceptions::ThrowTypeError("Object.defineProperty called on non-object");
    }
    Handle<JSPropertyKey> P = Conversion::ToPropertyKey(GetArg(args, 1));
    PropertyDescriptor desc = Objects::ToPropertyDescriptor(GetArg(args, 2));
    Objects::DefinePropertyOrThrow(O, P, desc);
    return O;
}

//Handle<JSValue> BuiltinObject::freeze(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
//}

Handle<JSValue> BuiltinObject::getOwnPropertyDescriptor(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSObject> obj = Conversion::ToObject(GetArg(args, 0));
    Handle<JSPropertyKey> key = Conversion::ToPropertyKey(GetArg(args, 1));
    return Objects::FromPropertyDescriptor(obj->GetOwnProperty(key));
}

//Handle<JSValue> BuiltinObject::getOwnPropertyNames(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
//}
//
//Handle<JSValue> BuiltinObject::getOwnPropertySymbols(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
//}

Handle<JSValue> BuiltinObject::getPrototypeOf(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSValue> O = GetArg(args, 0);
    switch (O->GetType()) {
        case JSValue::Type::kUndefined:
        case JSValue::Type::kNull:
            Exceptions::ThrowTypeError("Object.getPrototypeOf called on null or undefined");
        case JSValue::Type::kBoolean:
            return Context::CurrentRealm()->BooleanPrototype();
        case JSValue::Type::kNumber:
            return Context::CurrentRealm()->NumberPrototype();
        case JSValue::Type::kString:
            return Context::CurrentRealm()->StringPrototype();
        case JSValue::Type::kSymbol:
            return Context::CurrentRealm()->SymbolPrototype();
        default:
            Handle<JSObject> result = O.CastTo<JSObject>()->GetPrototypeOf();
            if (!result) {
                return JSNull::New();
            }
            return result;
    }
}

Handle<JSValue> BuiltinObject::is(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSValue> value1 = GetArg(args, 0);
    Handle<JSValue> value2 = GetArg(args, 1);
    return JSBoolean::New(Testing::SameValue(value1, value2));
}

Handle<JSValue> BuiltinObject::isExtensible(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSValue> OAsValue = GetArg(args, 0);
    if (!Testing::Is<JSObject>(OAsValue)) {
        return OAsValue;
    }
    return JSBoolean::New(OAsValue.CastTo<JSObject>()->IsExtensible());
}

//Handle<JSValue> BuiltinObject::isFrozen(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
//}
//
//Handle<JSValue> BuiltinObject::isSealed(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
//}
//
//Handle<JSValue> BuiltinObject::keys(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
//}
//
Handle<JSValue> BuiltinObject::preventExtensions(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSValue> OAsValue = GetArg(args, 0);
    if (!Testing::Is<JSObject>(OAsValue)) {
        return OAsValue;
    }
    bool status = OAsValue.CastTo<JSObject>()->PreventExtensions();
    if (!status) {
        Exceptions::ThrowTypeError("Object.preventExtensions failed");
    }
    return OAsValue;
}

//Handle<JSValue> BuiltinObject::seal(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
//}

Handle<JSValue> BuiltinObject::setPrototypeOf(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSValue> O = GetArg(args, 0);
    if (!O || O->GetType() == JSValue::Type::kNull) {
        Exceptions::ThrowTypeError("Object.setPrototypeOf called on null or undefined");
    }
    Handle<JSValue> proto = GetArg(args, 1);
    if (proto->GetType() == JSValue::Type::kNull) {
        proto = nullptr;
    } else if (!Testing::Is<JSObject>(proto)) {
        Exceptions::ThrowTypeError("Object prototype may only be object or null");
    }
    if (!Testing::Is<JSObject>(O)) {
        return O;
    }
    bool status = O.CastTo<JSObject>()->SetPrototypeOf(proto.CastTo<JSObject>());
    if (!status)
        Exceptions::ThrowTypeError("Object is not extensible or cyclic prototyping is found");
    return O;
}

Handle<JSValue> BuiltinObject::prototype::hasOwnProperty(const Handle<JSValue>& self, const Handle<Array<JSValue>>& args) {
    Handle<JSPropertyKey> P = Conversion::ToPropertyKey(GetArg(args, 0));
    Handle<JSObject> O = Conversion::ToObject(self);
    return JSBoolean::New(Objects::HasOwnProperty(O, P));
}

Handle<JSValue> BuiltinObject::prototype::isPrototypeOf(const Handle<JSValue>& self, const Handle<Array<JSValue>>& args) {
    Handle<JSValue> VAsValue = GetArg(args, 0);
    if (!Testing::Is<JSObject>(VAsValue)) {
        return JSBoolean::New(false);
    }
    Handle<JSObject> O = Conversion::ToObject(self);
    Handle<JSObject> V = VAsValue.CastTo<JSObject>();
    while (true) {
        V = V->GetPrototypeOf();
        if (!V) return JSBoolean::New(false);
        if (Testing::SameValue(O, V)) return JSBoolean::New(true);
    }
}

Handle<JSValue> BuiltinObject::prototype::propertyIsEnumerable(const Handle<JSValue>& self, const Handle<Array<JSValue>>& args) {
    Handle<JSPropertyKey> P = Conversion::ToPropertyKey(GetArg(args, 0));
    Handle<JSObject> O = Conversion::ToObject(self);
    Optional<PropertyDescriptor> desc = O->GetOwnProperty(P);
    if (!desc) return JSBoolean::New(false);
    return JSBoolean::New(*desc->enumerable);
}

Handle<JSValue> BuiltinObject::prototype::toLocaleString(const Handle<JSValue>& O, const Handle<Array<JSValue>>&) {
    return Objects::Invoke(O, "toString");
}

Handle<JSValue> BuiltinObject::prototype::toString(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    const char* builtinTag = nullptr;

    // Fast path for primitives
    switch (that->GetType()) {
        case JSValue::Type::kUndefined:
            builtinTag = "[object Undefined]";
            break;
        case JSValue::Type::kNull:
            builtinTag = "[object Null]";
            break;
        case JSValue::Type::kBoolean:
            builtinTag = "[object Boolean]";
            break;
        case JSValue::Type::kNumber:
            builtinTag = "[object Number]";
            break;
        case JSValue::Type::kString:
            builtinTag = "[object String]";
            break;
        default:
            break;
    }
    if (builtinTag) {
        return JSString::New(builtinTag);
    }

    builtinTag = "Object";
    // If type is Symbol, we simply skip this step
    // This is the step for special objects
    if (Testing::Is<JSObject>(that)) {
        Handle<JSObject> O = that.CastTo<JSObject>();

        // 4. Let isArray be IsArray(O).
        // 5. ReturnIfAbrupt(isArray).
        // 6. If isArray is true, let builtinTag be "Array".
        // 7. Else
        if (O.ExactInstanceOf<StringObject>()) {
            builtinTag = "String";
        }
        // 8. Else, if O has an[[ParameterMap]] internal slot, let builtinTag be "Arguments".
        else if (O->IsCallable()) {
            builtinTag = "Function";
        } else if (O.ExactInstanceOf<ErrorObject>()) {
            builtinTag = "Error";
        } else if (O.ExactInstanceOf<BooleanObject>()) {
            builtinTag = "Boolean";
        } else if (O.ExactInstanceOf<NumberObject>()) {
            builtinTag = "Number";
        } else if (O.ExactInstanceOf<DateObject>()) {
            builtinTag = "Date";
        } else if (O.ExactInstanceOf<RegExpObject>()) {
            builtinTag = "RegExp";
        }
    }

    // GetV is used here so that Symbol can be dealed as well
    Handle<JSValue> tag = Objects::GetV(that, JSSymbol::ToStringTag());

    // If tag is string, retrieve the string and use builtinTag otherwise
    Handle<JSString> tagAsString;
    if (Testing::Is<JSString>(tag)) {
        tagAsString = tag.CastTo<JSString>();
    } else {
        tagAsString = JSString::New(builtinTag);
    }

    // Concat the string and return the result
    return JSString::Concat({ JSString::New("[object "), tagAsString, JSString::New("]") });
}

Handle<JSValue> BuiltinObject::prototype::valueOf(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    return Conversion::ToObject(that);
}

Handle<JSValue> BuiltinObject::prototype::get__proto__(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    switch (that->GetType()) {
        case JSValue::Type::kUndefined:
        case JSValue::Type::kNull:
            Exceptions::ThrowIncompatibleReceiverTypeError("Object.prototype.get __proto__");
        case JSValue::Type::kBoolean:
            return Context::CurrentRealm()->BooleanPrototype();
        case JSValue::Type::kNumber:
            return Context::CurrentRealm()->NumberPrototype();
        case JSValue::Type::kString:
            return Context::CurrentRealm()->StringPrototype();
        case JSValue::Type::kSymbol:
            return Context::CurrentRealm()->SymbolPrototype();
        default:
            Handle<JSObject> result = that.CastTo<JSObject>()->GetPrototypeOf();
            if (!result) {
                return JSNull::New();
            }
            return result;
    }
}

Handle<JSValue> BuiltinObject::prototype::set__proto__(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    if (!that || that->GetType() == JSValue::Type::kNull) {
        Exceptions::ThrowIncompatibleReceiverTypeError("Object.prototype.set __proto__");
    } else if (!Testing::Is<JSObject>(that)) {
        return nullptr;
    }
    Handle<JSValue> proto = GetArg(args, 0);
    if (proto->GetType() == JSValue::Type::kNull) {
        proto = nullptr;
    } else if (!Testing::Is<JSObject>(proto)) {
        return nullptr;
    }
    bool status = that.CastTo<JSObject>()->SetPrototypeOf(proto.CastTo<JSObject>());
    if (!status)
        Exceptions::ThrowTypeError("Object is not extensible or cyclic prototyping is found");
    return nullptr;
}