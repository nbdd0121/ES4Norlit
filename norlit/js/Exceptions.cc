#include "Exception.h"

#include "../util/Arrays.h"

#include "Objects.h"
#include "vm/Context.h"
#include "vm/Realm.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::util;

void Exceptions::ThrowIncompatibleReceiverTypeError(const char* name) {
    Handle<JSString> msg = JSString::Concat(JSString::New(name), JSString::New(" called on incompatible receiver"));
    Handle<JSObject> error = Objects::Construct(Context::CurrentRealm()->TypeError(), Arrays::ToArray<JSValue>(msg));

    throw ESException(error);
}

void Exceptions::ThrowTypeError(const char* name) {
    Handle<JSString> msg = JSString::New(name);
    Handle<JSObject> error = Objects::Construct(Context::CurrentRealm()->TypeError(), Arrays::ToArray<JSValue>(msg));

    throw ESException(error);
}

void Exceptions::ThrowSyntaxError(const char* name) {
    Handle<JSString> msg = JSString::New(name);
    Handle<JSObject> error = Objects::Construct(Context::CurrentRealm()->SyntaxError(), Arrays::ToArray<JSValue>(msg));

    throw ESException(error);
}

void Exceptions::ThrowRangeError(const char* name) {
    Handle<JSString> msg = JSString::New(name);
    Handle<JSObject> error = Objects::Construct(Context::CurrentRealm()->RangeError(), Arrays::ToArray<JSValue>(msg));

    throw ESException(error);
}

void Exceptions::ThrowReferenceError(const Handle<JSString>& name) {
    Handle<JSString> msg = JSString::Concat(name, JSString::New(" is not defined"));
    Handle<JSObject> error = Objects::Construct(Context::CurrentRealm()->ReferenceError(), Arrays::ToArray<JSValue>(msg));

    throw ESException(error);
}