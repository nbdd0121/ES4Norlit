#include "../all.h"

#include "Realm.h"

#include "../../gc/Heap.h"
#include "../object/JSOrdinaryObject.h"
#include "../object/JSFunction.h"
#include "../builtin/Function.h"
#include "../builtin/Builtin.h"

#include "../JSSymbol.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::util;
using namespace norlit::js::builtin;

namespace {

PropertyDescriptor CreateDefaultDesc(const Handle<JSValue>& val) {
    return{
        val,
        nullopt,
        nullopt,
        true,
        false,
        true
    };
}

PropertyDescriptor ReadonlyDesc(const Handle<JSValue>& val) {
    return{
        val,
        nullopt,
        nullopt,
        false,
        false,
        true
    };
}

Handle<JSObject> CreateBuiltinFunction(
    const Handle<Realm>& realm, ESNativeFunction::CallFunc call, ESNativeFunction::CtorFunc ctor, const Handle<JSPropertyKey>& name, int argc
) {
    Handle<JSObject> obj = new ESNativeFunction(realm->FunctionPrototype(), realm, call, ctor);
    Objects::SetFunctionName(obj, name);
    Objects::DefinePropertyOrThrow(obj, JSString::New("length"), PropertyDescriptor{
        { JSNumber::New(argc) },
        nullopt,
        nullopt,
        false,
        false,
        true
    });
    return obj;
}

Handle<JSObject> CreateBuiltinFunction(
    const Handle<Realm>& realm, ESNativeFunction::CallFunc call, ESNativeFunction::CtorFunc ctor, const char* name, int argc
) {
    return CreateBuiltinFunction(realm, call, ctor, JSString::New(name), argc);
}

void DefineMethod(const Handle<Realm>& realm, const Handle<JSObject>& object, ESNativeFunction::CallFunc call, const Handle<JSPropertyKey>& name, int argc) {
    Objects::DefinePropertyOrThrow(object, name, CreateDefaultDesc(CreateBuiltinFunction(realm, call, nullptr, name, argc)));
}

void DefineMethod(const Handle<Realm>& realm, const Handle<JSObject>& object, ESNativeFunction::CallFunc call, const char* name, int argc) {
    DefineMethod(realm, object, call, JSString::New(name), argc);
}

void DefineProperty(const Handle<JSObject>& object, const Handle<JSPropertyKey>& name, const Handle<JSValue>& val) {
    Objects::DefinePropertyOrThrow(object, name, CreateDefaultDesc(val));
}

void DefineProperty(const Handle<JSObject>& object, const char* name, const Handle<JSValue>& val) {
    DefineProperty(object, JSString::New(name), val);
}

void DefineAccessorProperty(const Handle<Realm>& realm, const Handle<JSObject>& object, const Handle<JSPropertyKey>& name, ESNativeFunction::CallFunc getter, ESNativeFunction::CallFunc setter) {
    Handle<JSObject> getterObj = getter ? CreateBuiltinFunction(realm, getter, nullptr, name, 0) : nullptr;
    Handle<JSObject> setterObj = setter ? CreateBuiltinFunction(realm, setter, nullptr, name, 1) : nullptr;
    Objects::DefinePropertyOrThrow(object, name, PropertyDescriptor{
        nullopt,
        getterObj,
        setterObj,
        nullopt,
        false,
        true
    });
}

void DefineAccessorProperty(const Handle<Realm>& realm, const Handle<JSObject>& object, const char* name, ESNativeFunction::CallFunc getter, ESNativeFunction::CallFunc setter) {
    DefineAccessorProperty(realm, object, JSString::New(name), getter, setter);
}

void DefineFixedProperty(const Handle<JSObject>& object, const Handle<JSPropertyKey>& P, const Handle<JSValue>& val) {
    Objects::DefinePropertyOrThrow(object, P, PropertyDescriptor{
        val,
        nullopt,
        nullopt,
        false,
        false,
        false
    });
}

void DefineFixedProperty(const Handle<JSObject>& object, const char* name, const Handle<JSValue>& val) {
    Objects::DefinePropertyOrThrow(object, JSString::New(name), PropertyDescriptor{
        val,
        nullopt,
        nullopt,
        false,
        false,
        false
    });
}

void DefineReadonlyProperty(const Handle<JSObject>& object, const Handle<JSPropertyKey>& P, const Handle<JSValue>& val) {
    Objects::DefinePropertyOrThrow(object, P, PropertyDescriptor{
        val,
        nullopt,
        nullopt,
        false,
        false,
        true
    });
}

void DefineReadonlyProperty(const Handle<JSObject>& object, const char* name, const Handle<JSValue>& val) {
    DefineReadonlyProperty(object, JSString::New(name), val);
}

}

#define TODO(msg) [](const Handle<JSValue>&, const Handle<Array<JSValue>>&)->Handle<JSValue>{\
	throw #msg " has not been implemented yet";\
}

#define TODO_CTOR(msg) [](const Handle<Array<JSValue>>&, const Handle<JSObject>&)->Handle<JSObject>{\
	throw #msg " has not been implemented yet";\
}


Realm::Realm() {
    NoGC _;
    Handle<JSObject> objProto = new JSOrdinaryObject(nullptr);
    this->WriteBarrier<JSObject>(&this->ObjectPrototype_, objProto);
    // 5. Let throwerSteps be the algorithm steps specified in 9.2.7.1 for the %ThrowTypeError% function.
    // 6. Let thrower be CreateBuiltinFunction(realmRec, throwerSteps, null).
    // 7. Set intrinsics.[[%ThrowTypeError%]] to thrower.
    // 8. Let noSteps be an empty sequence of algorithm steps.
    Handle<JSObject> funcProto = new ESNativeFunction(objProto, this, Function::Prototype, nullptr);
    this->WriteBarrier<JSObject>(&this->FunctionPrototype_, funcProto);
    // 11. Call thrower.[[SetPrototypeOf]](funcProto).
    // 12. Perform AddRestrictedFunctionProperties(funcProto, realmRec)

    // 25.1 Iteration
    // Promoted because it has no dependency and is the dependency of %ArrayIteratorPrototype%
    if (true) {
        Handle<JSObject> prototype = Objects::ObjectCreate(objProto);
        this->WriteBarrier(&this->IteratorPrototype_, prototype);

        DefineMethod(this, prototype, [] (const Handle<JSValue>& that, const Handle<Array<JSValue>>&) -> Handle<JSValue> {
            return that;
        }, JSSymbol::Iterator(), 0);
    }

    // 18 The Global Object
    if (true) {
        // 18.2.1 eval
        this->WriteBarrier(&this->eval_, CreateBuiltinFunction(this, TODO(eval), nullptr, "eval", 1));
        // 18.2.2 isFinite
        this->WriteBarrier(&this->isFinite_, CreateBuiltinFunction(this, ::isFinite, nullptr, "isFinite", 1));
        // 18.2.3 isNaN
        this->WriteBarrier(&this->isNaN_, CreateBuiltinFunction(this, ::isNaN, nullptr, "isNaN", 1));
        // 18.2.4 parseFloat
        this->WriteBarrier(&this->parseFloat_, CreateBuiltinFunction(this, ::parseFloat, nullptr, "parseFloat", 2));
        // 18.2.5 parseInt
        this->WriteBarrier(&this->parseInt_, CreateBuiltinFunction(this, ::parseInt, nullptr, "parseInt", 2));
        // 18.2.6.2 decodeURI
        this->WriteBarrier(&this->decodeURI_, CreateBuiltinFunction(this, TODO(decodeURI), nullptr, "decodeURI", 1));
        // 18.2.6.3 decodeURIComponent
        this->WriteBarrier(&this->decodeURIComponent_, CreateBuiltinFunction(this, TODO(decodeURIComponent), nullptr, "decodeURIComponent", 1));
        // 18.2.6.5 encodeURI
        this->WriteBarrier(&this->encodeURI_, CreateBuiltinFunction(this, TODO(encodeURI), nullptr, "encodeURI", 1));
        // 18.2.6.6 encodeURIComponent
        this->WriteBarrier(&this->encodeURIComponent_, CreateBuiltinFunction(this, TODO(encodeURIComponent), nullptr, "encodeURIComponent", 1));
    }

    // 19.1 Object Objects
    if (true) {
        // 19.1.1 The Object Constructor
        Handle<JSObject> object = CreateBuiltinFunction(this, BuiltinObject::Call, BuiltinObject::Construct, "Object", 1);
        this->WriteBarrier(&this->Object_, object);

        // 19.1.2.1 Object.assign
        DefineMethod(this, object, TODO(Object.assign), "assign", 2);
        // 19.1.2.2 Object.create
        DefineMethod(this, object, BuiltinObject::create, "create", 2);
        // 19.1.2.3 Object.defineProperties
        DefineMethod(this, object, TODO(Object.defineProperties), "defineProperties", 2);
        // 19.1.2.4 Object.defineProperty
        DefineMethod(this, object, BuiltinObject::defineProperty, "defineProperty", 2);
        // 19.1.2.5 Object.freeze
        DefineMethod(this, object, TODO(Object.freeze), "freeze", 1);
        // 19.1.2.6 Object.getOwnPropertyDescriptor
        DefineMethod(this, object, BuiltinObject::getOwnPropertyDescriptor, "getOwnPropertyDescriptor", 2);
        // 19.1.2.7 Object.getOwnPropertyNames
        DefineMethod(this, object, TODO(Object.getOwnPropertyNames), "getOwnPropertyNames", 1);
        // 19.1.2.8 Object.getOwnPropertySymbols
        DefineMethod(this, object, TODO(Object.getOwnPropertySymbols), "getOwnPropertySymbols", 1);
        // 19.1.2.9 Object.getPrototypeOf
        DefineMethod(this, object, BuiltinObject::getPrototypeOf, "getPrototypeOf", 1);
        // 19.1.2.10 Object.is
        DefineMethod(this, object, BuiltinObject::is, "is", 2);
        // 19.1.2.11 Object.isExtensible
        DefineMethod(this, object, BuiltinObject::isExtensible, "isExtensible", 1);
        // 19.1.2.13 Object.isFrozen
        DefineMethod(this, object, TODO(Object.isFrozen), "isFrozen", 1);
        // 19.1.2.13 Object.isSealed
        DefineMethod(this, object, TODO(Object.isSealed), "isSealed", 1);
        // 19.1.2.14 Object.keys
        DefineMethod(this, object, TODO(Object.keys), "keys", 1);
        // 19.1.2.15 Object.preventExtensions
        DefineMethod(this, object, BuiltinObject::preventExtensions, "preventExtensions", 1);
        // 19.1.2.16 Object.prototype
        DefineFixedProperty(object, "prototype", objProto);
        // 19.1.2.17 Object.seal
        DefineMethod(this, object, TODO(Object.seal), "seal", 1);
        // 19.1.2.18 Object.setPrototypeOf
        DefineMethod(this, object, BuiltinObject::setPrototypeOf, "setPrototypeOf", 2);

        // 19.1.3.1 Object.prototype.constructor
        DefineProperty(objProto, "constructor", object);
        // 19.1.3.2 Object.prototype.hasOwnProperty
        DefineMethod(this, objProto, BuiltinObject::prototype::hasOwnProperty, "hasOwnProperty", 1);
        // 19.1.3.3 Object.prototype.isPrototypeOf
        DefineMethod(this, objProto, BuiltinObject::prototype::isPrototypeOf, "isPrototypeOf", 1);
        // 19.1.3.4 Object.prototype.propertyIsEnumerable
        DefineMethod(this, objProto, BuiltinObject::prototype::propertyIsEnumerable, "propertyIsEnumerable", 1);
        // 19.1.3.5 Object.prototype.toLocaleString
        DefineMethod(this, objProto, BuiltinObject::prototype::toLocaleString, "toLocaleString", 0);
        // 19.1.3.6 Object.prototype.toString
        Handle<JSObject> objProto_toString = CreateBuiltinFunction(this, BuiltinObject::prototype::toString, nullptr, "toString", 1);
        this->WriteBarrier(&this->ObjProto_toString_, objProto_toString);
        DefineProperty(objProto, "toString", objProto_toString);
        // 19.1.3.7 Object.prototype.valueOf
        DefineMethod(this, objProto, BuiltinObject::prototype::valueOf, "valueOf", 0);
    }

    // 19.2 Function Objects
    if (true) {
        // 19.1.1 The Function Constructor
        Handle<JSObject> object = CreateBuiltinFunction(this, Function::Call, Function::Construct, "Function", 1);
        this->WriteBarrier(&this->Function_, object);

        // 19.2.2.2
        DefineFixedProperty(object, "prototype", funcProto);

        // 19.2.3 Properties of the Function Prototype Object
        Objects::SetFunctionName(funcProto, "");
        Objects::DefinePropertyOrThrow(funcProto, JSString::New("length"), PropertyDescriptor{
            { JSNumber::New(0) },
            nullopt,
            nullopt,
            false,
            false,
            true
        });
        // 19.2.3.1 Function.prototype.apply
        DefineMethod(this, funcProto, Function::prototype::apply, "apply", 2);
        // 19.2.3.2 Function.prototype.bind
        DefineMethod(this, funcProto, Function::prototype::bind, "bind", 1);
        // 19.2.3.3 Function.prototype.call
        DefineMethod(this, funcProto, Function::prototype::call, "call", 1);
        // 19.2.3.4 Function.prototype.constructor
        DefineProperty(funcProto, "constructor", object);
        // 19.2.3.5 Function.prototype.toString
        DefineMethod(this, funcProto, Function::prototype::toString, "toString", 0);
        // 19.2.3.6 Function.prototype[@@hasInstance]
        DefineFixedProperty(funcProto, JSSymbol::HasInstance(), CreateBuiltinFunction(this,Function::prototype::Symbol_hasInstance, nullptr, "[Symbol.hasInstance]", 1));
    }

    // 19.3 Boolean Objects
    if (true) {
        // 19.3.1 The Boolean Constructor
        Handle<JSObject> object = CreateBuiltinFunction(this, Boolean::Call, Boolean::Construct, "Boolean", 1);
        this->WriteBarrier(&this->Boolean_, object);
        // 19.3.3 Properties of the Boolean Prototype Object
        Handle<JSObject> prototype = new JSOrdinaryObject(objProto);
        this->WriteBarrier(&this->BooleanPrototype_, prototype);

        // 19.3.2.1 Boolean.prototype
        DefineFixedProperty(object, "prototype", prototype);

        // 19.3.3.1 Boolean.prototype.constructor
        DefineProperty(prototype, "constructor", object);
        // 19.3.3.2 Boolean.prototype.toString
        DefineMethod(this, prototype, Boolean::prototype::toString, "toString", 0);
        // 19.3.3.3 Boolean.prototype.valueOf
        DefineMethod(this, prototype, Boolean::prototype::valueOf, "valueOf", 0);
    }

    // 19.4 Symbol Objects
    if (true) {
        // 19.4.1 The Symbol Constructor
        Handle<JSObject> object = CreateBuiltinFunction(this, Symbol::Call, nullptr, "Symbol", 1);
        this->WriteBarrier(&this->Symbol_, object);
        // 19.4.3 Properties of the Symbol Prototype Object
        Handle<JSObject> prototype = new JSOrdinaryObject(objProto);
        this->WriteBarrier(&this->SymbolPrototype_, prototype);

        // 19.4.2.1 Symbol.for
        DefineMethod(this, object, Symbol::for_, "for", 1);
        // 19.4.2.2 Symbol.hasInstance
        DefineFixedProperty(object, "hasInstance", JSSymbol::HasInstance());
        // 19.4.2.3 Symbol.isConcatSpreadable
        DefineFixedProperty(object, "isConcatSpreadable", JSSymbol::IsConcatSpreadable());
        // 19.4.2.4 Symbol.iterator
        DefineFixedProperty(object, "iterator", JSSymbol::Iterator());
        // 19.4.2.5 Symbol.keyFor
        DefineMethod(this, object, Symbol::keyFor, "keyFor", 1);
        // 19.4.2.6 Symbol.match
        DefineFixedProperty(object, "match", JSSymbol::Match());
        // 19.4.2.7 Symbol.prototype
        DefineFixedProperty(object, "prototype", prototype);
        // 19.4.2.8 Symbol.replace
        DefineFixedProperty(object, "replace", JSSymbol::Replace());
        // 19.4.2.9 Symbol.search
        DefineFixedProperty(object, "search", JSSymbol::Search());
        // 19.4.2.10 Symbol.species
        DefineFixedProperty(object, "species", JSSymbol::Species());
        // 19.4.2.11 Symbol.split
        DefineFixedProperty(object, "split", JSSymbol::Split());
        // 19.4.2.12 Symbol.toPrimitive
        DefineFixedProperty(object, "toPrimitive", JSSymbol::ToPrimitive());
        // 19.4.2.13 Symbol.toStringTag
        DefineFixedProperty(object, "toStringTag", JSSymbol::ToStringTag());
        // 19.4.2.14 Symbol.unscopables
        DefineFixedProperty(object, "unscopables", JSSymbol::Unscopables());

        // 19.4.3.1 Symbol.prototype.constructor
        DefineProperty(prototype, "constructor", object);
        // 19.4.3.2 Symbol.prototype.toString
        DefineMethod(this, prototype, Symbol::prototype::toString, "toString", 0);
        // 19.4.3.3 Symbol.prototype.valueOf
        DefineMethod(this, prototype, Symbol::prototype::valueOf, "valueOf", 0);
        // 19.4.3.4 Symbol.prototype[@@toPrimitive]
        Objects::DefinePropertyOrThrow(
            prototype, JSSymbol::ToPrimitive(), ReadonlyDesc(
                CreateBuiltinFunction(this, Symbol::prototype::Symbol_toPrimitive, nullptr, "[Symbol.toPrimitive]", 1)
            ));
        // 19.4.3.5 Symbol.prototype[@@toStringTag]
        Objects::DefinePropertyOrThrow(prototype, JSSymbol::ToStringTag(), ReadonlyDesc(JSString::New("Symbol")));
    }

    // 19.5 Error Objects
    if (true) {
#define ERROR_DEFINITION_GENERATE(className) do{\
			Handle<JSObject> object = CreateBuiltinFunction(this, className::Call, className::Construct, #className, 1);\
			this->WriteBarrier(&this->className##_, object);\
			Handle<JSObject> prototype = new JSOrdinaryObject(objProto);\
			this->WriteBarrier(&this->className##Prototype_, prototype);\
			DefineFixedProperty(object, "prototype", prototype);\
			DefineProperty(prototype, "constructor", object);\
			DefineProperty(prototype, "message", JSString::New(""));\
			DefineProperty(prototype, "name", JSString::New(#className));\
			DefineMethod(this, prototype, className::prototype::toString, "toString", 0);\
		}while(0)

        // 19.5.1 - 19.5.4
        ERROR_DEFINITION_GENERATE(Error);
        // 19.5.5.1 EvalError
        ERROR_DEFINITION_GENERATE(EvalError);
        // 19.5.5.2 RangeError
        ERROR_DEFINITION_GENERATE(RangeError);
        // 19.5.5.3 ReferenceError
        ERROR_DEFINITION_GENERATE(ReferenceError);
        // 19.5.5.4 SyntaxError
        ERROR_DEFINITION_GENERATE(SyntaxError);
        // 19.5.5.5 TypeError
        ERROR_DEFINITION_GENERATE(TypeError);
        // 19.5.5.6 TypeError
        ERROR_DEFINITION_GENERATE(URIError);

#undef ERROR_DEFINITION_GENERATE
    }

    // 20.1 Number Objects
    if (true) {
        Handle<JSObject> object = CreateBuiltinFunction(this, Number::Call, Number::Construct, "Number", 1);
        this->WriteBarrier(&this->Number_, object);
        Handle<JSObject> prototype = new JSOrdinaryObject(objProto);
        this->WriteBarrier(&this->NumberPrototype_, prototype);

        DefineFixedProperty(object, "EPISILON", JSNumber::New(2.2204460492503130808472633361816e-16));
        DefineMethod(this, object, Number::isFinite, "isFinite", 1);
        DefineMethod(this, object, Number::isInteger, "isInteger", 1);
        DefineMethod(this, object, Number::isNaN, "isNaN", 1);
        DefineMethod(this, object, Number::isSafeInteger, "isSafeInteger", 1);
        DefineFixedProperty(object, "MAX_SAFE_INTEGER", JSNumber::New(9007199254740991));
        DefineFixedProperty(object, "MAX_VALUE", JSNumber::New(1.7976931348623157e308));
        DefineFixedProperty(object, "MIN_SAFE_INTEGER", JSNumber::New(-9007199254740991));
        DefineFixedProperty(object, "MIN_VALUE", JSNumber::New(5e-324));
        DefineFixedProperty(object, "NaN", JSNumber::NaN());
        DefineFixedProperty(object, "NEGATIVE_INFINITY", JSNumber::New(-std::numeric_limits<double>::infinity()));
        DefineProperty(object, "parseFloat", this->parseFloat_);
        DefineProperty(object, "parseInt", this->parseInt_);
        DefineFixedProperty(object, "POSITIVE_INFINITY", JSNumber::Infinity());
        DefineFixedProperty(object, "prototype", prototype);

        DefineProperty(prototype, "constructor", object);
        DefineMethod(this, prototype, TODO(Number::prototype::toExponential), "toExponential", 1);
        DefineMethod(this, prototype, TODO(Number::prototype::toFixed), "toFixed", 1);
        DefineMethod(this, prototype, TODO(Number::prototype::toLocaleString), "toLocaleString", 0);
        DefineMethod(this, prototype, TODO(Number::prototype::toPrecision), "toPrecision", 1);
        DefineMethod(this, prototype, TODO(Number::prototype::toString), "toString", 1);
        DefineMethod(this, prototype, Number::prototype::valueOf, "valueOf", 0);
    }

    // 20.2 The Math Object
    if (true) {
        Handle<JSObject> object = Objects::ObjectCreate(objProto);
        this->WriteBarrier(&this->Math_, object);

        DefineFixedProperty(object, "E", JSNumber::New(2.7182818284590452354));
        DefineFixedProperty(object, "LN10", JSNumber::New(2.302585092994046));
        DefineFixedProperty(object, "LN2", JSNumber::New(0.6931471805599453));
        DefineFixedProperty(object, "LOG10E", JSNumber::New(0.4342944819032518));
        DefineFixedProperty(object, "LOG2E", JSNumber::New(1.4426950408889634));
        DefineFixedProperty(object, "PI", JSNumber::New(3.1415926535897932));
        DefineFixedProperty(object, "SQRT1_2", JSNumber::New(0.7071067811865476));
        DefineFixedProperty(object, "SQRT2", JSNumber::New(1.4142135623730951));
        DefineFixedProperty(object, JSSymbol::ToStringTag(), JSString::New("Math"));

        DefineMethod(this, object, Math::abs, "abs", 1);
        DefineMethod(this, object, Math::acos, "acos", 1);
        DefineMethod(this, object, Math::acosh, "acosh", 1);
        DefineMethod(this, object, Math::asin, "asin", 1);
        DefineMethod(this, object, Math::asinh, "asinh", 1);
        DefineMethod(this, object, Math::atan, "atan", 1);
        DefineMethod(this, object, Math::atanh, "atanh", 1);
        DefineMethod(this, object, Math::atan2, "atan2", 2);
        DefineMethod(this, object, Math::cbrt, "cbrt", 1);
        DefineMethod(this, object, Math::ceil, "ceil", 1);
        DefineMethod(this, object, Math::clz32, "clz32", 1);
        DefineMethod(this, object, Math::cos, "cos", 1);
        DefineMethod(this, object, Math::cosh, "cosh", 1);
        DefineMethod(this, object, Math::exp, "exp", 1);
        DefineMethod(this, object, Math::expm1, "expm1", 1);
        DefineMethod(this, object, Math::floor, "floor", 1);
        DefineMethod(this, object, Math::fround, "fround", 1);
        DefineMethod(this, object, Math::hypot, "hypot", 2);
        DefineMethod(this, object, Math::imul, "imul", 2);
        DefineMethod(this, object, Math::log, "log", 1);
        DefineMethod(this, object, Math::log1p, "log1p", 1);
        DefineMethod(this, object, Math::log10, "log10", 1);
        DefineMethod(this, object, Math::log2, "log2", 1);
        DefineMethod(this, object, Math::max, "max", 2);
        DefineMethod(this, object, Math::min, "min", 2);
        DefineMethod(this, object, Math::pow, "pow", 2);
        DefineMethod(this, object, Math::random, "random", 0);
        DefineMethod(this, object, Math::round, "round", 1);
        DefineMethod(this, object, Math::sign, "sign", 1);
        DefineMethod(this, object, Math::sin, "sin", 1);
        DefineMethod(this, object, Math::sinh, "sinh", 1);
        DefineMethod(this, object, Math::sqrt, "sqrt", 1);
        DefineMethod(this, object, Math::tan, "tan", 1);
        DefineMethod(this, object, Math::tanh, "tanh", 1);
        DefineMethod(this, object, Math::trunc, "trunc", 1);
    }

    // 20.3 Date Objects
    if (true) {
        Handle<JSObject> object = CreateBuiltinFunction(this, Date::Call, Date::Construct, "Date", 7);
        this->WriteBarrier(&this->Date_, object);
        Handle<JSObject> prototype = new JSOrdinaryObject(objProto);
        this->WriteBarrier(&this->DatePrototype_, prototype);

        DefineMethod(this, object, Date::now, "now", 0);
        DefineMethod(this, object, TODO(Date::parse), "parse", 1);
        DefineFixedProperty(object, "prototype", prototype);
        DefineMethod(this, object, TODO(Date::UTC), "UTC", 7);

        DefineProperty(prototype, "constructor", object);
        DefineMethod(this, prototype, Date::prototype::getDate, "getDate", 0);
        DefineMethod(this, prototype, Date::prototype::getDay, "getDay", 0);
        DefineMethod(this, prototype, Date::prototype::getFullYear, "getFullYear", 0);
        DefineMethod(this, prototype, Date::prototype::getHours, "getHours", 0);
        DefineMethod(this, prototype, Date::prototype::getMilliseconds, "getMilliseconds", 0);
        DefineMethod(this, prototype, Date::prototype::getMonth, "getMonth", 0);
        DefineMethod(this, prototype, Date::prototype::getMinutes, "getMinutes", 0);
        DefineMethod(this, prototype, Date::prototype::getSeconds, "getSeconds", 0);
        DefineMethod(this, prototype, Date::prototype::getTime, "getTime", 0);
        DefineMethod(this, prototype, Date::prototype::getTimezoneOffset, "getTimezoneOffset", 0);
        DefineMethod(this, prototype, Date::prototype::getUTCDate, "getUTCDate", 0);
        DefineMethod(this, prototype, Date::prototype::getUTCDay, "getUTCDay", 0);
        DefineMethod(this, prototype, Date::prototype::getUTCFullYear, "getUTCFullYear", 0);
        DefineMethod(this, prototype, Date::prototype::getUTCHours, "getUTCHours", 0);
        DefineMethod(this, prototype, Date::prototype::getUTCMilliseconds, "getUTCMilliseconds", 0);
        DefineMethod(this, prototype, Date::prototype::getUTCMinutes, "getUTCMinutes", 0);
        DefineMethod(this, prototype, Date::prototype::getUTCMonth, "getUTCMonth", 0);
        DefineMethod(this, prototype, Date::prototype::getUTCSeconds, "getUTCSeconds", 0);
        DefineMethod(this, prototype, Date::prototype::getYear, "getYear", 0);
        DefineMethod(this, prototype, Date::prototype::setDate, "setDate", 1);
        DefineMethod(this, prototype, TODO(Date::prototype::setFullYear), "setFullYear", 3);
        DefineMethod(this, prototype, TODO(Date::prototype::setHours), "setHours", 4);
        DefineMethod(this, prototype, TODO(Date::prototype::setMilliseconds), "setMilliseconds", 1);
        DefineMethod(this, prototype, TODO(Date::prototype::setMinutes), "setMinutes", 3);
        DefineMethod(this, prototype, TODO(Date::prototype::setMonth), "setMonth", 2);
        DefineMethod(this, prototype, TODO(Date::prototype::setSeconds), "setSeconds", 2);
        DefineMethod(this, prototype, TODO(Date::prototype::setTime), "setTime", 1);
        DefineMethod(this, prototype, TODO(Date::prototype::setUTCDate), "setUTCDate", 1);
        DefineMethod(this, prototype, TODO(Date::prototype::setUTCFullYear), "setUTCFullYear", 3);
        DefineMethod(this, prototype, TODO(Date::prototype::setUTCHours), "setUTCHours", 4);
        DefineMethod(this, prototype, TODO(Date::prototype::setUTCMilliseconds), "setUTCMilliseconds", 1);
        DefineMethod(this, prototype, TODO(Date::prototype::setUTCMinutes), "setUTCMinutes", 3);
        DefineMethod(this, prototype, TODO(Date::prototype::setUTCMonth), "setUTCMonth", 2);
        DefineMethod(this, prototype, TODO(Date::prototype::setUTCSeconds), "setUTCSeconds", 2);
        DefineMethod(this, prototype, TODO(Date::prototype::setYear), "setYear", 1);
        DefineMethod(this, prototype, TODO(Date::prototype::toDateString), "toDateString", 0);
        DefineMethod(this, prototype, TODO(Date::prototype::toGMTString), "toGMTString", 0);
        DefineMethod(this, prototype, TODO(Date::prototype::toISOString), "toISOString", 0);
        DefineMethod(this, prototype, TODO(Date::prototype::toJSON), "toJSON", 1);
        DefineMethod(this, prototype, TODO(Date::prototype::toLocaleDateString), "toLocaleDateString", 0);
        DefineMethod(this, prototype, TODO(Date::prototype::toLocaleString), "toLocaleString", 0);
        DefineMethod(this, prototype, TODO(Date::prototype::toLocaleTimeString), "toLocaleTimeString", 0);
        DefineMethod(this, prototype, TODO(Date::prototype::toString), "toString", 0);
        DefineMethod(this, prototype, TODO(Date::prototype::toTimeString), "toTimeString", 0);
        DefineMethod(this, prototype, TODO(Date::prototype::toUTCString), "toUTCString", 0);
        DefineMethod(this, prototype, Date::prototype::valueOf, "valueOf", 0);
        Objects::DefinePropertyOrThrow(
            prototype, JSSymbol::ToPrimitive(), ReadonlyDesc(
                CreateBuiltinFunction(this, Date::prototype::Symbol_toPrimitive, nullptr, "[Symbol.toPrimitive]", 1)
            ));
    }

    // 21.1 String Objects
    if (true) {
        Handle<JSObject> object = CreateBuiltinFunction(this, String::Call, String::Construct, "String", 1);
        this->WriteBarrier(&this->String_, object);
        Handle<JSObject> prototype = new JSOrdinaryObject(objProto);
        this->WriteBarrier(&this->StringPrototype_, prototype);

        DefineFixedProperty(object, "prototype", prototype);
        DefineMethod(this, object, TODO(String::fromCharCode), "fromCharCode", 1);
        DefineMethod(this, object, TODO(String::fromCodePoint), "fromCodePoint", 1);
        DefineMethod(this, object, TODO(String::raw), "raw", 1);

        DefineProperty(prototype, "constructor", object);
        DefineMethod(this, prototype, TODO(String::prototype::charAt), "charAt", 1);
        DefineMethod(this, prototype, TODO(String::prototype::charCodeAt), "charCodeAt", 1);
        DefineMethod(this, prototype, TODO(String::prototype::codePointAt), "codePointAt", 1);
        DefineMethod(this, prototype, TODO(String::prototype::concat), "concat", 1);
        DefineMethod(this, prototype, TODO(String::prototype::endsWith), "endsWith", 1);
        DefineMethod(this, prototype, TODO(String::prototype::includes), "includes", 1);
        DefineMethod(this, prototype, TODO(String::prototype::indexOf), "indexOf", 1);
        DefineMethod(this, prototype, TODO(String::prototype::lastIndexOf), "lastIndexOf", 1);
        DefineMethod(this, prototype, TODO(String::prototype::localCompare), "localCompare", 1);
        DefineMethod(this, prototype, TODO(String::prototype::match), "match", 1);
        DefineMethod(this, prototype, TODO(String::prototype::normalize), "normalize", 0);
        DefineMethod(this, prototype, TODO(String::prototype::repeat), "repeat", 1);
        DefineMethod(this, prototype, TODO(String::prototype::replace), "replace", 2);
        DefineMethod(this, prototype, TODO(String::prototype::search), "search", 1);
        DefineMethod(this, prototype, TODO(String::prototype::splice), "splice", 2);
        DefineMethod(this, prototype, TODO(String::prototype::split), "split", 2);
        DefineMethod(this, prototype, TODO(String::prototype::startsWith), "startsWith", 1);
        DefineMethod(this, prototype, TODO(String::prototype::substring), "substring", 2);
        DefineMethod(this, prototype, TODO(String::prototype::toLocaleLowerCase), "toLocaleLowerCase", 0);
        DefineMethod(this, prototype, TODO(String::prototype::toLocaleUpperCase), "toLocaleUpperCase", 0);
        DefineMethod(this, prototype, TODO(String::prototype::toLowerCase), "toLowerCase", 0);
        DefineMethod(this, prototype, TODO(String::prototype::toString), "toString", 0);
        DefineMethod(this, prototype, TODO(String::prototype::toUpperCase), "toUpperCase", 0);
        DefineMethod(this, prototype, TODO(String::prototype::trim), "trim", 0);
        DefineMethod(this, prototype, TODO(String::prototype::valueOf), "valueOf", 0);
        DefineMethod(this, prototype, TODO(String::prototype::Symbol_iterator), JSSymbol::Iterator(), 1);
        DefineMethod(this, prototype, TODO(String::prototype::substr), "substr", 2);
        DefineMethod(this, prototype, TODO(String::prototype::anchor), "anchor", 1);
        DefineMethod(this, prototype, TODO(String::prototype::big), "big", 0);
        DefineMethod(this, prototype, TODO(String::prototype::blink), "blink", 0);
        DefineMethod(this, prototype, TODO(String::prototype::bold), "bold", 0);
        DefineMethod(this, prototype, TODO(String::prototype::fixed), "fixed", 0);
        DefineMethod(this, prototype, TODO(String::prototype::fontcolor), "fontcolor", 0);
        DefineMethod(this, prototype, TODO(String::prototype::fontsize), "fontsize", 0);
        DefineMethod(this, prototype, TODO(String::prototype::italics), "italics", 0);
        DefineMethod(this, prototype, TODO(String::prototype::link), "link", 1);
        DefineMethod(this, prototype, TODO(String::prototype::small), "small", 0);
        DefineMethod(this, prototype, TODO(String::prototype::strike), "strike", 0);
        DefineMethod(this, prototype, TODO(String::prototype::sub), "sub", 0);
        DefineMethod(this, prototype, TODO(String::prototype::sup), "sup", 0);
    }

    // 21.2 RegExp Objects
    if (true) {
        Handle<JSObject> object = CreateBuiltinFunction(this, TODO(RegExp::Call), TODO_CTOR(RegExp::Construct), "RegExp", 2);
        this->WriteBarrier(&this->RegExp_, object);
        Handle<JSObject> prototype = new JSOrdinaryObject(objProto);
        this->WriteBarrier(&this->RegExpPrototype_, prototype);

        DefineFixedProperty(object, "prototype", prototype);
        DefineAccessorProperty(this, object, JSSymbol::Species(), TODO(RegExp::get_Symbol_species), nullptr);

        DefineProperty(prototype, "constructor", object);
        DefineMethod(this, prototype, TODO(RegExp::prototype::exec), "exec", 1);
        DefineAccessorProperty(this, prototype, "flags", TODO(RegExp::prototype::get_flags), nullptr);
        DefineAccessorProperty(this, prototype, "global", TODO(RegExp::prototype::get_global), nullptr);
        DefineAccessorProperty(this, prototype, "ignoreCase", TODO(RegExp::prototype::get_ignoreCase), nullptr);
        DefineMethod(this, prototype, TODO(RegExp::prototype::Symbol_match), JSSymbol::Match(), 1);
        DefineAccessorProperty(this, prototype, "multiline", TODO(RegExp::prototype::get_multiline), nullptr);
        DefineMethod(this, prototype, TODO(RegExp::prototype::Symbol_replace), JSSymbol::Replace(), 2);
        DefineMethod(this, prototype, TODO(RegExp::prototype::Symbol_search), JSSymbol::Search(), 1);
        DefineAccessorProperty(this, prototype, "source", TODO(RegExp::prototype::get_source), nullptr);
        DefineMethod(this, prototype, TODO(RegExp::prototype::Symbol_split), JSSymbol::Split(), 2);
        DefineAccessorProperty(this, prototype, "sticky", TODO(RegExp::prototype::get_sticky), nullptr);
        DefineMethod(this, prototype, TODO(RegExp::prototype::test), "test", 1);
        DefineMethod(this, prototype, TODO(RegExp::prototype::toString), "toString", 0);
        DefineAccessorProperty(this, prototype, "unicode", TODO(RegExp::prototype::get_unicode), nullptr);
    }

    // 22.1 Array Objects
    if (true) {
        Handle<JSObject> object = CreateBuiltinFunction(this, BuiltinArray::Call, BuiltinArray::Construct, "Array", 1);
        this->WriteBarrier(&this->Array_, object);
        Handle<JSObject> prototype = Objects::ArrayCreate(0, objProto);
        this->WriteBarrier(&this->ArrayPrototype_, prototype);

        DefineFixedProperty(object, "prototype", prototype);
        DefineMethod(this, object, TODO(BuiltinArray::from), "from", 1);
        DefineMethod(this, object, TODO(BuiltinArray::isArray), "isArray", 1);
        DefineMethod(this, object, TODO(BuiltinArray::of), "of", 0);
        DefineAccessorProperty(this, object, JSSymbol::Species(), TODO(BuiltinArray::get_Symbol_species), nullptr);

        if(true) {
            Handle<JSObject> blackList = Objects::ObjectCreate(nullptr);
            Objects::CreateDataProperty(blackList, "copyWithin", JSBoolean::New(true));
            Objects::CreateDataProperty(blackList, "entries", JSBoolean::New(true));
            Objects::CreateDataProperty(blackList, "fill", JSBoolean::New(true));
            Objects::CreateDataProperty(blackList, "find", JSBoolean::New(true));
            Objects::CreateDataProperty(blackList, "findIndex", JSBoolean::New(true));
            Objects::CreateDataProperty(blackList, "keys", JSBoolean::New(true));
            Objects::CreateDataProperty(blackList, "values", JSBoolean::New(true));
            DefineReadonlyProperty(prototype, JSSymbol::Unscopables(), blackList);
        }
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::concat), "concat", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::copyWithin), "copyWithin", 2);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::entries), "entries", 0);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::every), "every", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::fill), "fill", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::filter), "filter", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::find), "find", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::findIndex), "findIndex", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::forEach), "forEach", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::indexOf), "indexOf", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::join), "join", 1);
        DefineMethod(this, prototype, BuiltinArray::prototype::keys, "keys", 0);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::lastIndexOf), "lastIndexOf", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::map), "map", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::pop), "pop", 0);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::push), "push", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::reduce), "reduce", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::reduceRight), "reduceRight", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::reverse), "reverse", 0);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::shift), "shift", 0);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::slice), "slice", 2);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::some), "some", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::sort), "sort", 1);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::splice), "splice", 2);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::toLocaleString), "toLocaleString", 0);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::toString), "toString", 0);
        DefineMethod(this, prototype, TODO(BuiltinArray::prototype::unshift), "unshift", 1);

        Handle<JSObject> values = CreateBuiltinFunction(this, BuiltinArray::prototype::values, nullptr, "values", 0);
        DefineProperty(prototype, "values", values);
        DefineProperty(prototype, JSSymbol::Iterator(), values);

        Handle<JSObject> iteratorPrototype = Objects::ObjectCreate(this->IteratorPrototype_);
        this->WriteBarrier(&this->ArrayIteratorPrototype_, iteratorPrototype);

        DefineReadonlyProperty(iteratorPrototype, JSSymbol::ToStringTag(), JSString::New("Array Iterator"));
        DefineMethod(this, iteratorPrototype, BuiltinArray::Iterator_next, "next", 0);
    }

    // 25.2 GeneratorFunction Objects
    // 25.3 Generator Objects
    // Merged due to inter-dependency
    if (true) {
        Handle<JSObject> object = CreateBuiltinFunction(this, TODO(GeneratorFunction::Call), TODO_CTOR(GeneratorFunction::Construct), "GeneratorFunction", 1);
        this->WriteBarrier(&this->GeneratorFunction_, object);
        Handle<JSObject> prototype = Objects::ObjectCreate(funcProto);
        this->WriteBarrier(&this->Generator_, prototype);
        Handle<JSObject> genProto = Objects::ObjectCreate(this->IteratorPrototype_);
        this->WriteBarrier(&this->GeneratorPrototype_, genProto);

        DefineFixedProperty(object, "prototype", prototype);

        DefineReadonlyProperty(prototype, "constructor", object);
        DefineReadonlyProperty(prototype, "prototype", genProto);
        DefineReadonlyProperty(prototype, JSSymbol::ToStringTag(), JSString::New("GeneratorFunction"));

        DefineReadonlyProperty(genProto, "constructor", prototype);
        DefineReadonlyProperty(genProto, JSSymbol::ToStringTag(), JSString::New("Generator"));
        DefineMethod(this, genProto, Generator::prototype::next, "next", 1);
        DefineMethod(this, genProto, Generator::prototype::return_, "return", 1);
        DefineMethod(this, genProto, Generator::prototype::throw_, "throw", 1);
    }

    // B.2 Additional Builtin Properties
    if (true) {
        // B.2.1.1 escape
        this->WriteBarrier(&this->escape_, CreateBuiltinFunction(this, TODO(escape), nullptr, "escape", 1));
        // B.2.1.2 unescape
        this->WriteBarrier(&this->unescape_, CreateBuiltinFunction(this, TODO(unescape), nullptr, "unescape", 1));
        // B.2.2.1 Object.prototype.__proto__
        Objects::DefinePropertyOrThrow(objProto, "__proto__", {
            nullopt,
            CreateBuiltinFunction(this, BuiltinObject::prototype::get__proto__, nullptr, "get __proto__", 0),
            CreateBuiltinFunction(this, BuiltinObject::prototype::set__proto__, nullptr, "set __proto__", 1),
            nullopt,
            false,
            true
        });
    }

    // Conforming to SetRealmGlobalObject(this, undefined)
    Handle<JSObject> globalObj = new JSOrdinaryObject(objProto);
    this->WriteBarrier(&this->globalThis_, globalObj);
    Handle<GlobalEnvironment> newGlobalEnv = new GlobalEnvironment(globalObj);
    this->WriteBarrier(&this->globalEnv_, newGlobalEnv);

    // Conforming to SetDefaultGlobalBindings(this)
    if(true) {
        // 18.1.1 Infinity
        DefineFixedProperty(globalObj, "Infinity", JSNumber::Infinity());
        // 18.1.2 NaN
        DefineFixedProperty(globalObj, "NaN", JSNumber::NaN());
        // 18.1.3 undefined
        DefineFixedProperty(globalObj, "undefined", nullptr);

        // 18.2.1 eval(x)
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("eval"), CreateDefaultDesc(this->eval_));
        // 18.2.2 isFinite(number)
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("isFinite"), CreateDefaultDesc(this->isFinite_));
        // 18.2.3 isNaN(number)
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("isNaN"), CreateDefaultDesc(this->isNaN_));
        // 18.2.4 parseFloat(string)
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("parseFloat"), CreateDefaultDesc(this->parseFloat_));
        // 18.2.5 parseInt(string, radix)
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("parseInt"), CreateDefaultDesc(this->parseInt_));
        // 18.2.6.2 decodeURI(encodedURI)
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("decodeURI"), CreateDefaultDesc(this->decodeURI_));
        // 18.2.6.3 decodeURIComponent(encodedURIComponent)
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("decodeURIComponent"), CreateDefaultDesc(this->decodeURIComponent_));
        // 18.2.6.5 encodeURI(uri)
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("encodeURI"), CreateDefaultDesc(this->encodeURI_));
        // 18.2.6.6 encodeURIComponent(uriComponent)
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("encodeURIComponent"), CreateDefaultDesc(this->encodeURIComponent_));
        // B.2.1.1 escape
        DefineProperty(globalObj, "escape", this->escape_);
        // B.2.1.2 unescape
        DefineProperty(globalObj, "unescape", this->unescape_);

        // 18.3.1 Array
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Array"), CreateDefaultDesc(this->Array_));
        // 18.3.2 ArrayBuffer
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("ArrayBuffer"), CreateDefaultDesc(this->ArrayBuffer_));
        // 18.3.3 Boolean
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Boolean"), CreateDefaultDesc(this->Boolean_));
        // 18.3.4 DataView
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("DataView"), CreateDefaultDesc(this->DataView_));
        // 18.3.5 Date
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Date"), CreateDefaultDesc(this->Date_));
        // 18.3.6 Error
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Error"), CreateDefaultDesc(this->Error_));
        // 18.3.7 EvalError
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("EvalError"), CreateDefaultDesc(this->EvalError_));
        // 18.3.8 Float32Array
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Float32Array"), CreateDefaultDesc(this->Float32Array_));
        // 18.3.9 Float64Array
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Float64Array"), CreateDefaultDesc(this->Float64Array_));
        // 18.3.10 Function
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Function"), CreateDefaultDesc(this->Function_));
        // 18.3.11 Int8Array
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Int8Array"), CreateDefaultDesc(this->Int8Array_));
        // 18.3.12 Int16Array
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Int16Array"), CreateDefaultDesc(this->Int16Array_));
        // 18.3.13 Int32Array
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Int32Array"), CreateDefaultDesc(this->Int32Array_));
        // 18.3.14 Map
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Map"), CreateDefaultDesc(this->Map_));
        // 18.3.15 Number
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Number"), CreateDefaultDesc(this->Number_));
        // 18.3.16 Object
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Object"), CreateDefaultDesc(this->Object_));
        // 18.3.17 Proxy
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Proxy"), CreateDefaultDesc(this->Proxy_));
        // 18.3.18 Promise
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Promise"), CreateDefaultDesc(this->Promise_));
        // 18.3.19 RangeError
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("RangeError"), CreateDefaultDesc(this->RangeError_));
        // 18.3.20 ReferenceError
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("ReferenceError"), CreateDefaultDesc(this->ReferenceError_));
        // 18.3.21 RegExp
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("RegExp"), CreateDefaultDesc(this->RegExp_));
        // 18.3.22 Set
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Set"), CreateDefaultDesc(this->Set_));
        // 18.3.23 String
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("String"), CreateDefaultDesc(this->String_));
        // 18.3.24 Symbol
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Symbol"), CreateDefaultDesc(this->Symbol_));
        // 18.3.25 SyntaxError
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("SyntaxError"), CreateDefaultDesc(this->SyntaxError_));
        // 18.3.26 TypeError
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("TypeError"), CreateDefaultDesc(this->TypeError_));
        // 18.3.27 Uint8Array
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Uint8Array"), CreateDefaultDesc(this->Uint8Array_));
        // 18.3.28 Uint8ClampedArray
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Uint8ClampedArray"), CreateDefaultDesc(this->Uint8ClampedArray_));
        // 18.3.29 Uint16Array
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Uint16Array"), CreateDefaultDesc(this->Uint16Array_));
        // 18.3.30 Uint32Array
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Uint32Array"), CreateDefaultDesc(this->Uint32Array_));
        // 18.3.31 URIError
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("URIError"), CreateDefaultDesc(this->URIError_));
        // 18.3.32 WeakMap
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("WeakMap"), CreateDefaultDesc(this->WeakMap_));
        // 18.3.33 WeakSet
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("WeakSet"), CreateDefaultDesc(this->WeakSet_));

        // 18.4.1 JSON
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("JSON"), CreateDefaultDesc(this->JSON_));
        // 18.4.2 Math
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Math"), CreateDefaultDesc(this->Math_));
        // 18.4.3 Reflect
        Objects::DefinePropertyOrThrow(globalObj, JSString::New("Reflect"), CreateDefaultDesc(this->Reflect_));
    }
}

void Realm::IterateField(const FieldIterator& iter) {
    iter(&this->Array_);
    iter(&this->ArrayBuffer_);
    iter(&this->ArrayBufferPrototype_);
    iter(&this->ArrayIteratorPrototype_);
    iter(&this->ArrayPrototype_);
    iter(&this->ArrayProto_values_);
    iter(&this->Boolean_);
    iter(&this->BooleanPrototype_);
    iter(&this->DataView_);
    iter(&this->DataViewPrototype_);
    iter(&this->Date_);
    iter(&this->DatePrototype_);
    iter(&this->decodeURI_);
    iter(&this->decodeURIComponent_);
    iter(&this->encodeURI_);
    iter(&this->encodeURIComponent_);
    iter(&this->Error_);
    iter(&this->ErrorPrototype_);
    iter(&this->escape_);
    iter(&this->eval_);
    iter(&this->EvalError_);
    iter(&this->EvalErrorPrototype_);
    iter(&this->Float32Array_);
    iter(&this->Float32ArrayPrototype_);
    iter(&this->Float64Array_);
    iter(&this->Float64ArrayPrototype_);
    iter(&this->Function_);
    iter(&this->FunctionPrototype_);
    iter(&this->Generator_);
    iter(&this->GeneratorFunction_);
    iter(&this->GeneratorPrototype_);
    iter(&this->Int8Array_);
    iter(&this->Int8ArrayPrototype_);
    iter(&this->Int16Array_);
    iter(&this->Int16ArrayPrototype_);
    iter(&this->Int32Array_);
    iter(&this->Int32ArrayPrototype_);
    iter(&this->isFinite_);
    iter(&this->isNaN_);
    iter(&this->IteratorPrototype_);
    iter(&this->JSON_);
    iter(&this->Map_);
    iter(&this->MapIteratorPrototype_);
    iter(&this->MapPrototype_);
    iter(&this->Math_);
    iter(&this->Number_);
    iter(&this->NumberPrototype_);
    iter(&this->Object_);
    iter(&this->ObjectPrototype_);
    iter(&this->ObjProto_toString_);
    iter(&this->parseFloat_);
    iter(&this->parseInt_);
    iter(&this->Promise_);
    iter(&this->PromisePrototype_);
    iter(&this->Proxy_);
    iter(&this->RangeError_);
    iter(&this->RangeErrorPrototype_);
    iter(&this->ReferenceError_);
    iter(&this->ReferenceErrorPrototype_);
    iter(&this->Reflect_);
    iter(&this->RegExp_);
    iter(&this->RegExpPrototype_);
    iter(&this->Set_);
    iter(&this->SetIteratorPrototype_);
    iter(&this->SetPrototype_);
    iter(&this->String_);
    iter(&this->StringIteratorPrototype_);
    iter(&this->StringPrototype_);
    iter(&this->Symbol_);
    iter(&this->SymbolPrototype_);
    iter(&this->SyntaxError_);
    iter(&this->SyntaxErrorPrototype_);
    iter(&this->ThrowTypeError_);
    iter(&this->TypedArray_);
    iter(&this->TypedArrayPrototype_);
    iter(&this->TypeError_);
    iter(&this->TypeErrorPrototype_);
    iter(&this->Uint8Array_);
    iter(&this->Uint8ArrayPrototype_);
    iter(&this->Uint8ClampedArray_);
    iter(&this->Uint8ClampedArrayPrototype_);
    iter(&this->Uint16Array_);
    iter(&this->Uint16ArrayPrototype_);
    iter(&this->Uint32Array_);
    iter(&this->Uint32ArrayPrototype_);
    iter(&this->unescape_);
    iter(&this->URIError_);
    iter(&this->URIErrorPrototype_);
    iter(&this->WeakMap_);
    iter(&this->WeakMapPrototype_);
    iter(&this->WeakSet_);
    iter(&this->WeakSetPrototype_);

    iter(&this->globalThis_);
    iter(&this->globalEnv_);
}


