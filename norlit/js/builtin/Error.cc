#include "Builtin.h"

#include "../vm/Context.h"
#include "../vm/Realm.h"

#include "../Objects.h"
#include "../Testing.h"
#include "../Conversion.h"

#include "../object/Exotics.h"

#include "../Exception.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::builtin;
using namespace norlit::util;

namespace {

Handle<JSString> ErrorToString(const Handle<JSObject>& O, const char* ctorName) {
    Handle<JSString> name;
    if (Handle<JSValue> nameAsValue = Objects::Get(O, "name")) {
        name = Conversion::ToString(nameAsValue);
    } else {
        name = JSString::New(ctorName);
    }
    Handle<JSString> msg;
    if (Handle<JSValue> msgAsValue = Objects::Get(O, "message")) {
        msg = Conversion::ToString(msgAsValue);
    } else {
        msg = JSString::New("");
    }
    if (name->Length() == 0) {
        return msg;
    }
    if (msg->Length() == 0) {
        return name;
    }
    return JSString::Concat({ name, JSString::New(": "), msg });
}

Handle<JSObject> ConstructErrorObject(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target, Realm::Intrinsic proto, const char* ctorName) {
    static int reentry = 0;
    reentry ++;
    Handle<JSValue> message = GetArg(args, 0);
    Handle<JSObject> O = Objects::OrdinaryCreateFromConstructor<ErrorObject>(target, proto);
    if (message) {
        message = Conversion::ToString(message);
        Objects::DefinePropertyOrThrow(O, JSString::New("message"), PropertyDescriptor{
            message,
            nullopt,
            nullopt,
            true,
            false,
            true
        });
    }


    Handle<JSString> stackString = ErrorToString(O, ctorName);
    ArrayList<Context>& ctx = Context::GetContextStack();

    if (reentry == 1) {
        std::wstring stackBuilder = &stackString->ToWChar()->At(0);

        for (size_t size = ctx.Size(), i = size - 2; i < size; i--) {
            Handle<Context> c = ctx.Get(i);

            Handle<JSValue> that = c->thisPointer();
            Handle<ESFunctionBase> func = c->function();
            if (that&&!Testing::Is<JSNull>(that)) {
                try {
                    Handle<JSValue> ctor = Objects::GetV(that, "constructor");
                    if (!ctor) {
                        goto fallback;
                    }
                    Handle<JSString> ctorName = Conversion::ToString(Objects::GetV(ctor, "name"));
                    Handle<JSString> funcName = Conversion::ToString(Objects::GetV(func, "name"));
                    stackBuilder += L"\n    at ";
                    stackBuilder += &ctorName->ToWChar()->At(0);
                    stackBuilder += '.';
                    stackBuilder += &funcName->ToWChar()->At(0);
                } catch (ESException&) {
                    goto fallback;
                }
                continue;
            }

fallback:
            if (func) {
                Handle<JSString> funcName = Conversion::ToString(Objects::GetV(func, "name"));
                stackBuilder += L"\n    at ";
                stackBuilder += &funcName->ToWChar()->At(0);
            } else {
                stackBuilder += L"\n    at script";
            }
        }

        stackString = JSString::New(stackBuilder.c_str());
    } else {
        stackString = JSString::Concat(stackString, JSString::New("\n    Due to Error re-entrance, stack information is not displayed"));
    }

    Objects::DefinePropertyOrThrow(O, JSString::New("stack"), PropertyDescriptor{
        { stackString },
        nullopt,
        nullopt,
        true,
        false,
        true
    });

    return O;
}

}

#define ERROR_DEFINITION_GENERATE(className) \
Handle<JSValue> className::Call(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {\
    return Construct(args, Context::CurrentContext()->function());\
}\
Handle<JSObject> className::Construct(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {\
    return ConstructErrorObject(args, target, &Realm::className##Prototype, #className);\
}\
Handle<JSValue> className::prototype::toString(const Handle<JSValue>& O, const Handle<Array<JSValue>>&) {\
    if (!Testing::Is<JSObject>(O)) {\
		Exceptions::ThrowIncompatibleReceiverTypeError(#className ".prototype.call");\
    }\
    return ErrorToString(O.CastTo<JSObject>(), #className);\
}

ERROR_DEFINITION_GENERATE(Error)
ERROR_DEFINITION_GENERATE(EvalError)
ERROR_DEFINITION_GENERATE(RangeError)
ERROR_DEFINITION_GENERATE(ReferenceError)
ERROR_DEFINITION_GENERATE(SyntaxError)
ERROR_DEFINITION_GENERATE(TypeError)
ERROR_DEFINITION_GENERATE(URIError)

#undef ERROR_DEFINITION_GENERATE
