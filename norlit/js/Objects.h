#ifndef NORLIT_JS_OBJECTS_H
#define NORLIT_JS_OBJECTS_H

#include "object/JSOrdinaryObject.h"

#include "vm/Realm.h"

#include "JSString.h"

#include "../util/Arrays.h"

namespace norlit {
namespace js {

enum class FunctionKind: uint8_t {
    kNormal,
    kMethod,
    kArrow,
    kClassConstructor,
    kNonConstructor,
    kGenerator
};

namespace object {
class ESFunction;
}

namespace bytecode {
class Code;
}

class Objects {
  public:

    // 6.2.4.4
    static gc::Handle<object::JSObject> FromPropertyDescriptor(const util::Optional<object::PropertyDescriptor>&);

    // 6.2.4.5
    static object::PropertyDescriptor ToPropertyDescriptor(const gc::Handle<JSValue>&);
    static object::PropertyDescriptor ToPropertyDescriptor(const gc::Handle<object::JSObject>&);

    // 7.3.1
    static gc::Handle<JSValue> Get(const gc::Handle<object::JSObject>&, const gc::Handle<JSPropertyKey>&);
    static gc::Handle<JSValue> Get(const gc::Handle<object::JSObject>& obj, const char* name) {
        return Get(obj, JSString::New(name));
    }

    // 7.3.2
    static gc::Handle<JSValue> GetV(const gc::Handle<JSValue>&, const gc::Handle<JSPropertyKey>&);
    static gc::Handle<JSValue> GetV(const gc::Handle<JSValue>& obj, const char* name) {
        return GetV(obj, JSString::New(name));
    }

    // 7.3.3
    static bool Set(const gc::Handle<object::JSObject>&, const gc::Handle<JSPropertyKey>&, const gc::Handle<JSValue>&, bool);
    static bool Set(const gc::Handle<object::JSObject>& O, const char* P, const gc::Handle<JSValue>& V, bool Throw) {
        return Set(O, JSString::New(P), V, Throw);
    }

    // 7.3.4
    static bool CreateDataProperty(const gc::Handle<object::JSObject>&, const gc::Handle<JSPropertyKey>&, const gc::Handle<JSValue>&);
    static bool CreateDataProperty(const gc::Handle<object::JSObject>& O, const char* P, const gc::Handle<JSValue>& V) {
        return CreateDataProperty(O, JSString::New(P), V);
    }

    // 7.3.5
    static bool CreateDataPropertyOrThrow(const gc::Handle<object::JSObject>&, const gc::Handle<JSPropertyKey>&, const gc::Handle<JSValue>&);
    static bool CreateDataPropertyOrThrow(const gc::Handle<object::JSObject>& O, const char* P, const gc::Handle<JSValue>& V) {
        return CreateDataPropertyOrThrow(O, JSString::New(P), V);
    }

    // 7.3.7
    static void DefinePropertyOrThrow(const gc::Handle<object::JSObject>&, const gc::Handle<JSPropertyKey>&, const object::PropertyDescriptor&);
    static void DefinePropertyOrThrow(const gc::Handle<object::JSObject>& O, const char* P, const object::PropertyDescriptor& desc) {
        DefinePropertyOrThrow(O, JSString::New(P), desc);
    }

    // 7.3.9
    static gc::Handle<object::JSObject> GetMethod(const gc::Handle<JSValue>&, const gc::Handle<JSPropertyKey>&);
    static gc::Handle<object::JSObject> GetMethod(const gc::Handle<JSValue>& O, const char* P) {
        return GetMethod(O, JSString::New(P));
    }

    // 7.3.10
    static bool HasProperty(const gc::Handle<object::JSObject>&, const gc::Handle<JSPropertyKey>&);
    static bool HasProperty(const gc::Handle<object::JSObject>& O, const char* P) {
        return HasProperty(O, JSString::New(P));
    }

    // 7.3.11
    static bool HasOwnProperty(const gc::Handle<object::JSObject>&, const gc::Handle<JSPropertyKey>&);
    static bool HasOwnProperty(const gc::Handle<object::JSObject>& O, const char* P) {
        return HasOwnProperty(O, JSString::New(P));
    }

    // 7.3.12
    static gc::Handle<JSValue> Call(const gc::Handle<object::JSObject>&, const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> Call(const gc::Handle<object::JSObject>& F, const gc::Handle<JSValue>& V) {
        return Call(F, V, util::Arrays::ToArray<JSValue>());
    }

    // 7.3.13
    static gc::Handle<object::JSObject> Construct(const gc::Handle<object::JSObject>&, const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<object::JSObject>&);
    static gc::Handle<object::JSObject> Construct(const gc::Handle<object::JSObject>& F, const gc::Handle<gc::Array<JSValue>>& argumentList) {
        return Construct(F, argumentList, F);
    }

    // 7.3.14
    static bool SetIntegrityLevel_sealed(const gc::Handle<object::JSObject>&);
    static bool SetIntegrityLevel_frozen(const gc::Handle<object::JSObject>&);

    // 7.3.15
    static bool TestIntegrityLevel_sealed(const gc::Handle<object::JSObject>&);
    static bool TestIntegrityLevel_frozen(const gc::Handle<object::JSObject>&);

    // 7.3.16
    static gc::Handle<object::JSObject> CreateArrayFromList(const gc::Handle<gc::Array<JSValue>>&);

    // 7.3.17
    static gc::Handle<gc::Array<JSValue>> CreateListFromArrayLike(const gc::Handle<JSValue>&);

    // 7.3.18
    static gc::Handle<JSValue> Invoke(const gc::Handle<JSValue>&, const gc::Handle<JSPropertyKey>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> Invoke(const gc::Handle<JSValue>& O, const gc::Handle<JSPropertyKey>& P) {
        return Invoke(O, P, util::Arrays::ToArray<JSValue>());
    }
    static gc::Handle<JSValue> Invoke(const gc::Handle<JSValue>& O, const char* P, const gc::Handle<gc::Array<JSValue>>& args) {
        return Invoke(O, JSString::New(P), args);
    }
    static gc::Handle<JSValue> Invoke(const gc::Handle<JSValue>& O, const char* P) {
        return Invoke(O, JSString::New(P));
    }

    // 7.3.19
    static bool OrdinaryHasInstance(const gc::Handle<JSValue>&, const gc::Handle<JSValue>&);

    // 7.3.22
    static gc::Handle<vm::Realm> GetFunctionRealm(const gc::Handle<object::JSObject>&);

    // 9.1.13
    template<typename T = object::JSOrdinaryObject>
    static gc::Handle<T> ObjectCreate(const gc::Handle<object::JSObject>& proto) {
        return new T(proto);
    }

    // 9.1.14
    template<typename T = object::JSOrdinaryObject>
    static gc::Handle<T> OrdinaryCreateFromConstructor(const gc::Handle<object::JSObject>& ctor, vm::Realm::Intrinsic defaultProto) {
        return ObjectCreate<T>(GetPrototypeFromConstructor(ctor, defaultProto));
    }

    // 9.1.15
    static gc::Handle<object::JSObject> GetPrototypeFromConstructor(const gc::Handle<object::JSObject>&, vm::Realm::Intrinsic);

    // 9.2.3
    static gc::Handle<object::ESFunction> FunctionAllocate(const gc::Handle<object::JSObject>&, bool, FunctionKind = FunctionKind::kNormal);

    // 9.2.4
    static void FunctionInitialize(
        const gc::Handle<object::ESFunction>&, FunctionKind, const gc::Handle<bytecode::Code>&, const gc::Handle<vm::Environment>&);

    // 9.2.5
    static gc::Handle<object::JSObject> FunctionCreate(FunctionKind, const gc::Handle<bytecode::Code>&, const gc::Handle<vm::Environment>&, bool, const gc::Handle<object::JSObject>& = nullptr);

    // 9.2.5
    static gc::Handle<object::JSObject> GeneratorFunctionCreate(FunctionKind, const gc::Handle<bytecode::Code>&, const gc::Handle<vm::Environment>&, bool);

    // 9.2.8
    static void MakeConstructor(const gc::Handle<object::JSObject>&, bool, const gc::Handle<object::JSObject>&);
    static void MakeConstructor(const gc::Handle<object::JSObject>&, bool = true);

    // 9.2.11
    static void SetFunctionName(const gc::Handle<object::JSObject>&, const gc::Handle<JSPropertyKey>&, const char* = nullptr);
    static void SetFunctionName(const gc::Handle<object::JSObject>& F, const char* name, const char* prefix = nullptr) {
        SetFunctionName(F, JSString::New(name), prefix);
    }

    // 9.4.2.2
    static gc::Handle<object::JSObject> ArrayCreate(int64_t, const gc::Handle<object::JSObject>& = nullptr);

    // 9.4.3.4
    static gc::Handle<object::JSObject> StringCreate(const gc::Handle<JSString>&, const gc::Handle<object::JSObject>&);


    // 12.9.4
    static bool InstanceOfOperator(const gc::Handle<JSValue>&, const gc::Handle<JSValue>&);
};

}
}

#endif


