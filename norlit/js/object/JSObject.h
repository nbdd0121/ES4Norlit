#ifndef NORLIT_JS_OBJECT_JSOBJECT_H
#define NORLIT_JS_OBJECT_JSOBJECT_H

#include "Property.h"
#include "../JSValue.h"
#include "../../util/Optional.h"
#include "../../gc/Array.h"

namespace norlit {
namespace js {

class JSPropertyKey;

namespace object {

class ESFunctionBase;

class JSObject : public JSValue {
  public:
    virtual Type VirtualGetType() const override;

    virtual gc::Handle<JSObject> GetPrototypeOf() = 0;
    virtual bool SetPrototypeOf(const gc::Handle<JSObject>&) = 0;
    virtual bool IsExtensible() = 0;
    virtual bool PreventExtensions() = 0;
    virtual util::Optional<PropertyDescriptor> GetOwnProperty(const gc::Handle<JSPropertyKey>&) = 0;
    virtual bool DefineOwnProperty(const gc::Handle<JSPropertyKey>&, const PropertyDescriptor&) = 0;
    virtual bool HasProperty(const gc::Handle<JSPropertyKey>&) = 0;
    virtual gc::Handle<JSValue> Get(const gc::Handle<JSPropertyKey>&, const gc::Handle<JSValue>&) = 0;
    virtual bool Set(const gc::Handle<JSPropertyKey>&, const gc::Handle<JSValue>&, const gc::Handle<JSValue>&) = 0;
    virtual bool Delete(const gc::Handle<JSPropertyKey>&) = 0;

    virtual gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    virtual gc::Handle<JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<JSObject>&);
    virtual bool IsCallable();
    virtual bool IsConstructor();
    // Enumerate
    virtual gc::Handle<gc::Array<JSPropertyKey>> OwnPropertyKeys() = 0;
};

}
}
}


#endif
