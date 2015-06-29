#ifndef NORLIT_JS_OBJECT_PROPERTY_H
#define NORLIT_JS_OBJECT_PROPERTY_H

#include "../../gc/Object.h"
#include "../../gc/Handle.h"
#include "../../util/Optional.h"

namespace norlit {
namespace js {

class JSValue;

namespace object {

class JSObject;

class Property : public gc::Object {
  public:
    Property() {}
    bool enumerable;
    bool configurable;

    virtual bool IsDataProperty() = 0;
    virtual bool IsAccessorProperty() = 0;
};

class DataProperty : public Property {
  public:
    DataProperty() {}

    JSValue* value;
    bool writable;

    virtual bool IsDataProperty() {
        return true;
    }
    virtual bool IsAccessorProperty() {
        return false;
    }

    virtual void IterateField(const gc::FieldIterator& callback) override;
};

class AccessorProperty : public Property {
  public:
    AccessorProperty() {}
    JSObject* get;
    JSObject* set;

    virtual bool IsDataProperty() {
        return false;
    }
    virtual bool IsAccessorProperty() {
        return true;
    }

    virtual void IterateField(const gc::FieldIterator& callback) override;
};

struct PropertyDescriptor {
    util::Optional<gc::Handle<JSValue>> value;
    util::Optional<gc::Handle<JSObject>> get;
    util::Optional<gc::Handle<JSObject>> set;
    util::Optional<bool> writable;
    util::Optional<bool> enumerable;
    util::Optional<bool> configurable;

    bool IsDataDescriptor() const {
        return value || writable;
    }

    bool IsAccessorDescriptor() const {
        return get || set;
    }

    bool IsGenericDescriptor() const {
        return !IsDataDescriptor() && !IsAccessorDescriptor();
    }
};

}
}
}

#endif