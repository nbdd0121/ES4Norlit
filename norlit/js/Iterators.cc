#include "all.h"

#include "vm/Context.h"

#include "../util/Arrays.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::object;
using namespace norlit::js::vm;
using namespace norlit::util;

Handle<JSObject> Iterators::GetIterator(const Handle<JSValue>& obj, const Handle<JSObject>& method) {
    Handle<JSValue> iterator = Objects::Call(method, obj);
    if (!Testing::Is<JSObject>(iterator)) {
        Exceptions::ThrowTypeError("Iterator must be object");
    }
    return iterator.CastTo<JSObject>();
}

Handle<JSObject> Iterators::GetIterator(const Handle<JSValue>& obj) {
    return GetIterator(obj, Objects::GetMethod(obj, JSSymbol::Iterator()));
}

Handle<JSObject> Iterators::IteratorNext(const Handle<JSObject>& obj, const Handle<JSValue>& value) {
    Handle<JSValue> ret = Objects::Invoke(obj, "next", Arrays::ToArray<JSValue>(value));
    if (!Testing::Is<JSObject>(ret)) {
        Exceptions::ThrowTypeError("Iteration result must be object");
    }
    return ret.CastTo<JSObject>();
}

Handle<JSObject> Iterators::IteratorNext(const Handle<JSObject>& obj) {
    Handle<JSValue> ret = Objects::Invoke(obj, "next");
    if (!Testing::Is<JSObject>(ret)) {
        Exceptions::ThrowTypeError("Iteration result must be object");
    }
    return ret.CastTo<JSObject>();
}

bool Iterators::IteratorComplete(const Handle<JSObject>& iterResult) {
    return Conversion::ToBooleanValue(Objects::Get(iterResult, "done"));
}

Handle<JSValue> Iterators::IteratorValue(const Handle<JSObject>& iterResult) {
    return Objects::Get(iterResult, "value");
}

Handle<JSObject> Iterators::IteratorStep(const Handle<JSObject>& iterator) {
    Handle<JSObject> result = IteratorNext(iterator);
    if (IteratorComplete(result)) {
        return nullptr;
    }
    return result;
}

Handle<JSObject> Iterators::CreateIterResultObject(const Handle<JSValue>& value, bool done) {
    Handle<JSObject> obj = Objects::ObjectCreate(Context::CurrentRealm()->ObjectPrototype());
    Objects::CreateDataProperty(obj, "value", value);
    Objects::CreateDataProperty(obj, "done", JSBoolean::New(done));
    return obj;
}