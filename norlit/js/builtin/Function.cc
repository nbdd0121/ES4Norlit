#include "Function.h"

#include "../JSString.h"
#include "../JSBoolean.h"

#include "../Testing.h"
#include "../Objects.h"
#include "../Conversion.h"
#include "../Exception.h"
#include "../object/JSFunction.h"

#include <typeinfo>


using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::object;
using namespace norlit::js::builtin;
using namespace norlit::util;

namespace {

Handle<JSValue> GetArg(const Handle<Array<JSValue>>& args, size_t index) {
    if (args->Length() <= index) {
        return nullptr;
    }
    return args->Get(index);
}

Handle<Array<JSValue>> GetRestArg(const Handle<Array<JSValue>>& args, size_t index) {
    if (args->Length() <= index) {
        return Array<JSValue>::New(0);
    }
    size_t size = args->Length() - index;;
    Handle<Array<JSValue>> ret = Array<JSValue>::New(size);
    for (size_t i = 0; i < size; i++) {
        ret->Put(i, args->Get(index + i));
    }
    return ret;
}

}

Handle<JSValue> Function::Call(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    throw "TODO";
}

Handle<JSObject> Function::Construct(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {
    throw "TODO";
}

Handle<JSValue> Function::Prototype(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
    return nullptr;
}

Handle<JSValue> Function::prototype::apply(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    if (!Testing::IsCallable(that)) {
        Exceptions::ThrowIncompatibleReceiverTypeError("Function.prototype.apply");
    }
    Handle<JSValue> thisArg = GetArg(args, 0);
    Handle<JSValue> argArray = GetArg(args, 1);
    if (!argArray || Testing::Is<JSNull>(argArray)) {
        return Objects::Call(that.CastTo<JSObject>(), thisArg);
    }
    Handle<Array<JSValue>> argList = Objects::CreateListFromArrayLike(argArray);
    // 4. Perform PrepareForTailCall().
    return Objects::Call(that.CastTo<JSObject>(), thisArg, argList);
}

Handle<JSValue> Function::prototype::bind(const Handle<JSValue>& that, const Handle<Array<JSValue>>& argumentsList) {
    if (!Testing::IsCallable(that)) {
        Exceptions::ThrowIncompatibleReceiverTypeError("Function.prototype.bind");
    }
    Handle<JSObject> Target = that.CastTo<JSObject>();
    Handle<JSValue> thisArg = GetArg(argumentsList, 0);
    Handle<Array<JSValue>> args = GetRestArg(argumentsList, 1);
    Handle<JSObject> F = BoundFunction::New(Target, thisArg, args);
    Handle<JSNumber> L = JSNumber::Zero();
    if (Objects::HasOwnProperty(Target, "length")) {
        Handle<JSValue> targetLen = Objects::Get(Target, "length");
        if (Handle<JSNumber> targetLenAsNum = Testing::CastIf<JSNumber>(targetLen)) {
            if (std::isfinite(targetLenAsNum->Value())) {
                int64_t L64 = Conversion::ToIntegerValue(targetLenAsNum) - args->Length();
                if (L64 > 0) {
                    L = JSNumber::New(L64);
                }
            } else if (targetLenAsNum->Value() == std::numeric_limits<double>::infinity()) {
                L = targetLenAsNum;
            }
        }
    }

    Objects::DefinePropertyOrThrow(F, JSString::New("length"), {
        { L },
        nullopt,
        nullopt,
        false,
        false,
        true
    });
    //	Assert: not an abrupt completion.

    Handle<JSString> targetName = Testing::CastIf<JSString>(Objects::Get(Target, "name"));
    if (!targetName) {
        targetName = JSString::New("");
    }

    Objects::SetFunctionName(F, targetName, "bound");
    return F;
}

Handle<JSValue> Function::prototype::call(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    if (!Testing::IsCallable(that)) {
        Exceptions::ThrowIncompatibleReceiverTypeError("Function.prototype.call");
    }
    Handle<JSValue> thisArg = GetArg(args, 0);
    Handle<Array<JSValue>> argList = GetRestArg(args, 1);
    // 4. Perform PrepareForTailCall().
    return Objects::Call(that.CastTo<JSObject>(), thisArg, argList);
}

Handle<JSValue> Function::prototype::toString(const Handle<JSValue>& that, const Handle<Array<JSValue>>&) {
    if (!Testing::Is<JSObject>(that)) {
        Exceptions::ThrowIncompatibleReceiverTypeError("Function.prototype.toString");
    }
    const std::type_info& info = that.TypeId();
    if (info == typeid(ESNativeFunction) || info == typeid(BoundFunction)) {
        Handle<JSString> name = Conversion::ToString(Objects::Get(that.CastTo<JSObject>(), JSString::New("name")));
        return JSString::Concat({JSString::New("function "), name, JSString::New("() { [native code] }")});
    } else {
        throw "TODO";
    }
}

Handle<JSValue> Function::prototype::Symbol_hasInstance(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    return JSBoolean::New(Objects::OrdinaryHasInstance(that, GetArg(args, 0)));
}