#include "JSObject.h"
#include "JSFunction.h"
#include "../vm/Context.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;

JSValue::Type JSObject::VirtualGetType() const {
    return Type::kObject;
}

Handle<JSValue> JSObject::Call(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
    throw "Not callable";
}

Handle<JSObject> JSObject::Construct(const Handle<Array<JSValue>>&, const Handle<JSObject>&) {
    throw "Not constructor";
}

bool JSObject::IsCallable() {
    return false;
}

bool JSObject::IsConstructor() {
    return false;
}