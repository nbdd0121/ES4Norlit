#include "all.h"

#include "JSSymbol.h"

#include "object/Exotics.h"

#include "vm/Context.h"
#include "vm/Realm.h"

#include <limits>
#include <algorithm>

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::bytecode;
using namespace norlit::util;

Handle<JSObject> Objects::FromPropertyDescriptor(const Optional<PropertyDescriptor>& desc) {
    if (!desc) {
        return nullptr;
    }
    Handle<JSObject> obj = ObjectCreate(Context::CurrentRealm()->ObjectPrototype());
    if (desc->value) {
        CreateDataPropertyOrThrow(obj, "value", *desc->value);
    }
    if (desc->writable) {
        CreateDataPropertyOrThrow(obj, "writable", JSBoolean::New(*desc->writable));
    }
    if (desc->get) {
        CreateDataPropertyOrThrow(obj, "get", *desc->get);
    }
    if (desc->set) {
        CreateDataPropertyOrThrow(obj, "set", *desc->set);
    }
    if (desc->enumerable) {
        CreateDataPropertyOrThrow(obj, "enumerable", JSBoolean::New(*desc->enumerable));
    }
    if (desc->configurable) {
        CreateDataPropertyOrThrow(obj, "configurable", JSBoolean::New(*desc->configurable));
    }
    return obj;
}

PropertyDescriptor Objects::ToPropertyDescriptor(const Handle<JSValue>& Obj) {
    if (!Testing::Is<JSObject>(Obj)) {
        Exceptions::ThrowTypeError("Cannot convert a non-object to a property descriptor");
    }
    return ToPropertyDescriptor(Obj.CastTo<JSObject>());
}

PropertyDescriptor Objects::ToPropertyDescriptor(const Handle<JSObject>& Obj) {
    PropertyDescriptor desc;
    if (HasProperty(Obj, "enumerable")) {
        desc.enumerable = Conversion::ToBooleanValue(Get(Obj, "enumerable"));
    }
    if (HasProperty(Obj, "configurable")) {
        desc.configurable = Conversion::ToBooleanValue(Get(Obj, "configurable"));
    }
    if (HasProperty(Obj, "value")) {
        desc.value = Get(Obj, "value");
    }
    if (HasProperty(Obj, "writable")) {
        desc.writable = Conversion::ToBooleanValue(Get(Obj, "writable"));
    }
    if (HasProperty(Obj, "get")) {
        Handle<JSValue> getter = Get(Obj, "get");
        if (getter && !Testing::IsCallable(getter)) {
            Exceptions::ThrowTypeError("Getter either be a function or undefined");
        }
        desc.get = getter.CastTo<JSObject>();
    }
    if (HasProperty(Obj, "set")) {
        Handle<JSValue> setter = Get(Obj, "set");
        if (setter && !Testing::IsCallable(setter)) {
            Exceptions::ThrowTypeError("Setter either be a function or undefined");
        }
        desc.set = setter.CastTo<JSObject>();
    }
    if (desc.get || desc.set) {
        if (desc.value || desc.writable) {
            Exceptions::ThrowTypeError("Property descriptor cannot be both accessor property and a data property");
        }
    }
    return desc;
}

Handle<JSValue> Objects::Get(const Handle<JSObject>& obj, const Handle<JSPropertyKey>& key) {
    return obj->Get(key, obj);
}

Handle<JSValue> Objects::GetV(const Handle<JSValue>& obj, const Handle<JSPropertyKey>& key) {
    switch (obj->GetType()) {
        case JSValue::Type::kUndefined:
        case JSValue::Type::kNull:
            Exceptions::ThrowTypeError("TypeError: Cannot read property from null or undefined");
        case JSValue::Type::kBoolean:
            return Context::CurrentRealm()->BooleanPrototype()->Get(key, obj);
        case JSValue::Type::kNumber:
            return Context::CurrentRealm()->NumberPrototype()->Get(key, obj);
        case JSValue::Type::kString: {
            if (Handle<JSString> keyAsStr = Testing::CastIf<JSString>(key)) {
                Handle<JSString> self = obj.CastTo<JSString>();
                int64_t index = Conversion::ToIntegerIndex(keyAsStr);
                if (index != -1 && index < static_cast<int64_t>(self->Length())) {
                    return self->Substring(static_cast<size_t>(index), static_cast<size_t>(index+1));
                }
                if (keyAsStr == JSString::New("length")) {
                    return JSNumber::New(static_cast<int64_t>(self->Length()));
                }
            }
            return Context::CurrentRealm()->StringPrototype()->Get(key, obj);
        }
        case JSValue::Type::kSymbol:
            return Context::CurrentRealm()->SymbolPrototype()->Get(key, obj);
        default:
            return obj.CastTo<JSObject>()->Get(key, obj);
    }
}

bool Objects::Set(const Handle<JSObject>& O, const Handle<JSPropertyKey>& P, const Handle<JSValue>& V, bool Throw) {
    bool success = O->Set(P, V, O);
    if (!success && Throw) Exceptions::ThrowTypeError("Set value failed");
    return success;
}

bool Objects::CreateDataProperty(const Handle<JSObject>& O, const Handle<JSPropertyKey>& P, const Handle<JSValue>& V) {
    return O->DefineOwnProperty(P, {
        V,
        nullopt,
        nullopt,
        true,
        true,
        true
    });
}

bool Objects::CreateDataPropertyOrThrow(const Handle<JSObject>& O, const Handle<JSPropertyKey>& P, const Handle<JSValue>& V) {
    if (!CreateDataProperty(O, P, V)) {
        Exceptions::ThrowTypeError("Cannot create data property");
    }
    return true;
}

void Objects::DefinePropertyOrThrow(const Handle<JSObject>& O, const Handle<JSPropertyKey>& P, const PropertyDescriptor& desc) {
    bool success = O->DefineOwnProperty(P, desc);
    if (!success)
        Exceptions::ThrowTypeError("Define property failed");
}

Handle<JSObject> Objects::GetMethod(const Handle<JSValue>& O, const Handle<JSPropertyKey>& P) {
    Handle<JSValue> func = GetV(O, P);
    if (!func || func->GetType() == JSValue::Type::kNull) {
        return nullptr;
    }
    Handle<JSObject> obj = func.CastTo<JSObject>();
    if (!obj->IsCallable()) {
        Exceptions::ThrowTypeError("Method is not callable");
    }
    return obj;
}

bool Objects::HasProperty(const Handle<JSObject>& O, const Handle<JSPropertyKey>& P) {
    return O->HasProperty(P);
}

bool Objects::HasOwnProperty(const Handle<JSObject>& O, const Handle<JSPropertyKey>& P) {
    if (O->GetOwnProperty(P)) {
        return true;
    } else {
        return false;
    }
}

Handle<JSValue> Objects::Call(const Handle<JSObject>& func, const Handle<JSValue>& self, const Handle<Array<JSValue>>& args) {
    if (!Testing::IsCallable(func)) {
        Exceptions::ThrowTypeError("Cannot call on non-callable");
    }
    return func->Call(self, args);
}

Handle<JSObject> Objects::Construct(const Handle<JSObject>& func, const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {
    if (!Testing::IsConstructor(func)) {
        Exceptions::ThrowTypeError("Cannot new on non-constructor");
    }
    return func->Construct(args, target);
}

Handle<JSObject> Objects::CreateArrayFromList(const Handle<Array<JSValue>>& list) {
    size_t len = list->Length();
    Handle<JSObject> array = ArrayCreate(len);
    for (size_t i = 0; i < len; i++) {
        Handle<JSString> P = Conversion::ToString(JSNumber::New(static_cast<int64_t>(i)));
        CreateDataProperty(array, P, list->Get(i));
    }
    return array;
}

Handle<Array<JSValue>> Objects::CreateListFromArrayLike(const Handle<JSValue>& objAsVal) {
    Handle<JSObject> obj = Testing::CastIf<JSObject>(objAsVal);
    if (!obj)Exceptions::ThrowTypeError("Non-object is not array-like");
    int64_t len = Conversion::ToLength(Get(obj, "length"));
    Handle<Array<JSValue>> list = Array<JSValue>::New(static_cast<size_t>(len));
    for (int64_t index = 0; index < len; index++) {
        Handle<JSString> indexName = Conversion::ToString(JSNumber::New(index));
        Handle<JSValue> next = Get(obj, indexName);
        list->Put(static_cast<size_t>(index), next);
    }
    return list;
}

Handle<JSValue> Objects::Invoke(const Handle<JSValue>& O, const Handle<JSPropertyKey>& P, const Handle<Array<JSValue>>& argumentsList) {
    Handle<JSValue> func = GetV(O, P);
    if (!Testing::Is<JSObject>(func)) {
        Exceptions::ThrowTypeError("Cannot call on non-callable");
    }
    return Call(func.CastTo<JSObject>(), O, argumentsList);
}

bool Objects::OrdinaryHasInstance(const Handle<JSValue>& C, const Handle<JSValue>& O) {
    if (!Testing::IsCallable(C)) {
        return false;
    }
    if (Handle<BoundFunction> bf = C.ExactCheckedCastTo<BoundFunction>()) {
        return InstanceOfOperator(O, bf->boundTargetFunction());
    }
    if (!Testing::Is<JSObject>(O)) {
        return false;
    }
    Handle<JSValue> P = Get(C.CastTo<JSObject>(), "prototype");
    if (!Testing::Is<JSObject>(P)) {
        Exceptions::ThrowTypeError("Prototype of constructor must be object");
    }
    Handle<JSObject> OP = O.CastTo<JSObject>();
    while (true) {
        OP = OP->GetPrototypeOf();
        if (!OP) return false;
        if (P == OP)return true;
    }
}


Handle<Realm> Objects::GetFunctionRealm(const Handle<JSObject>& obj) {
    assert(Testing::IsCallable(obj));
    if (Handle<ESFunctionBase> func = obj.DynamicCastTo<ESFunctionBase>()) {
        return func->realm();
    }
    if (Handle<BoundFunction> func = obj.ExactCheckedCastTo<BoundFunction>()) {
        return GetFunctionRealm(func->boundTargetFunction());
    }
    throw "TODO: Objects::GetFunctionRealm";
    /*
    4. If obj is a Proxy exotic object, then
    	a.If the value of the[[ProxyHandler]] internal slot of obj is null, throw a TypeError exception.
    	b.Let proxyTarget be the value of obj¡¯s[[ProxyTarget]] internal slot.
    	c.Return GetFunctionRealm(proxyTarget).
    5. Return the running execution context¡¯s Realm
    */
}

Handle<JSObject> Objects::GetPrototypeFromConstructor(const Handle<JSObject>& ctor, Realm::Intrinsic defaultProto) {
    // TODO: Assert: IsConstructor(constructor) is true.
    Handle<JSValue> proto = Get(ctor, JSString::New("prototype"));
    if (Testing::Is<JSObject>(proto)) {
        return proto.CastTo<JSObject>();
    } else {
        return (GetFunctionRealm(ctor)->*defaultProto)();
    }
}

Handle<ESFunction> Objects::FunctionAllocate(const Handle<JSObject>& functionPrototype, bool strict, FunctionKind kind) {
    bool needConstruct = true;
    if (kind == FunctionKind::kNonConstructor) {
        kind = FunctionKind::kNormal;
        needConstruct = false;
    }
    Handle<ESFunction> F = new ESFunction(functionPrototype, Context::CurrentRealm());
    if (needConstruct) {
        if (kind==FunctionKind::kGenerator) {
            F->constructorKind(ESFunction::ConstructorKind::kDerived);
        } else {
            F->constructorKind(ESFunction::ConstructorKind::kBase);
        }
    } else {
        F->constructorKind(ESFunction::ConstructorKind::kNonConstructor);
    }
    F->strict(strict);
    F->functionKind(kind);
    return F;
}

void Objects::FunctionInitialize(
    const Handle<ESFunction>& F, FunctionKind kind, const Handle<bytecode::Code>& bytecode, const Handle<vm::Environment>& Scope) {
    int32_t len = 0;
    // 2. Let len be the ExpectedArgumentCount of ParameterList.
    Objects::DefinePropertyOrThrow(F, "length", {
        {JSNumber::New(len) },
        nullopt,
        nullopt,
        false,
        false,
        true
    });
    bool Strict = F->strict();
    F->environment(Scope);
    F->code(bytecode);
    if (kind == FunctionKind::kArrow) {
        F->thisMode(ESFunction::ThisMode::kLexical);
    } else {
        if (Strict) {
            F->thisMode(ESFunction::ThisMode::kStrict);
        } else {
            F->thisMode(ESFunction::ThisMode::kGlobal);
        }
    }
}


Handle<JSObject> Objects::FunctionCreate(FunctionKind kind, const Handle<Code>& bc, const Handle<Environment>& env, bool strict, const Handle<JSObject>& protoConst) {
    Handle<JSObject> proto = protoConst;
    if (!proto) {
        proto = Context::CurrentRealm()->FunctionPrototype();
    }
    FunctionKind allocKind = kind;
    if (kind != FunctionKind::kNormal) {
        allocKind = FunctionKind::kNonConstructor;
    };
    Handle<ESFunction> F = FunctionAllocate(proto, strict, allocKind);
    FunctionInitialize(F, kind, bc, env);
    return F;
}

Handle<JSObject> Objects::GeneratorFunctionCreate(FunctionKind kind, const Handle<Code>& bc, const Handle<Environment>& env, bool strict) {
    Handle<ESFunction> F = FunctionAllocate(Context::CurrentRealm()->Generator(), strict, FunctionKind::kGenerator);
    FunctionInitialize(F, kind, bc, env);
    return F;
}

void Objects::MakeConstructor(const Handle<JSObject>& F, bool writablePrototype, const Handle<JSObject>& prototype) {
    DefinePropertyOrThrow(F, "prototype", {
        { prototype },
        nullopt,
        nullopt,
        writablePrototype,
        false,
        true
    });
}

void Objects::MakeConstructor(const Handle<JSObject>& F, bool writablePrototype) {
    Handle<JSObject> prototype = ObjectCreate(Context::CurrentRealm()->ObjectPrototype());
    DefinePropertyOrThrow(prototype, "constructor", {
        { F },
        nullopt,
        nullopt,
        writablePrototype,
        false,
        true
    });
    MakeConstructor(F, writablePrototype, prototype);
}

void Objects::SetFunctionName(const Handle<JSObject>& F, const Handle<JSPropertyKey>& nameAsKey, const char* prefix) {
    Handle<JSString> name;
    if (Handle<JSSymbol> nameAsSym = Testing::CastIf<JSSymbol>(nameAsKey)) {
        Handle<JSString> description = nameAsSym->Descriptor();
        if (!description) {
            name = JSString::New("");
        } else {
            name = JSString::Concat({ JSString::New("["), description, JSString::New("]") });
        }
    } else {
        name = nameAsKey.CastTo<JSString>();
    }
    if (prefix) {
        name = JSString::Concat({ JSString::New(prefix), JSString::New(" "), name });
    }
    DefinePropertyOrThrow(F, "name", {
        {name},
        nullopt,
        nullopt,
        false,
        false,
        true
    });
    // Assert: the result is never an abrupt completion.
}

Handle<JSObject> Objects::ArrayCreate(int64_t len, const Handle<JSObject>& protoConst) {
    if (len < 0 || len>0xFFFFFFFF) {
        Exceptions::ThrowRangeError("Array length out of range");
    }
    Handle<JSObject> proto = protoConst;
    if (!proto) {
        proto = Context::CurrentRealm()->ArrayPrototype();
    }
    Handle<ArrayObject> ret = new ArrayObject(proto);
    JSOrdinaryObject::OrdinaryDefineOwnProperty(ret, JSString::New("length"), {
        { JSNumber::New(static_cast<int64_t>(len)) },
        nullopt,
        nullopt,
        true,
        false,
        false
    });
    return ret;
}

Handle<JSObject> Objects::StringCreate(const Handle<JSString>& value, const Handle<JSObject>& prototype) {
    Handle<StringObject> S = ObjectCreate<StringObject>(prototype);
    S->stringData(value);
    JSOrdinaryObject::OrdinaryDefineOwnProperty(S, JSString::New("length"), {
        { JSNumber::New(static_cast<int64_t>(value->Length())) },
        nullopt,
        nullopt,
        false,
        false,
        false
    });
    return S;
}

bool Objects::InstanceOfOperator(const Handle<JSValue>& O, const Handle<JSValue>& C) {
    if (!Testing::Is<JSObject>(C)) {
        Exceptions::ThrowTypeError("Constructor argument of instanceof operator must be object");
    }
    Handle<JSObject> instOfHandler = GetMethod(C.CastTo<JSObject>(), JSSymbol::HasInstance());
    if (instOfHandler) {
        return Conversion::ToBooleanValue(Call(instOfHandler, C, Arrays::ToArray<JSValue>(O)));
    }
    if (!Testing::IsCallable(C)) {
        Exceptions::ThrowTypeError("Constructor argument of instanceof operator must be callable");
    }
    return OrdinaryHasInstance(C, O);
}