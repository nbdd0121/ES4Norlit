#ifndef NORLIT_JS_OBJECT_JSORDINARYOBJECT_H
#define NORLIT_JS_OBJECT_JSORDINARYOBJECT_H

#include "JSObject.h"
#include "../../util/ArrayList.h"

namespace norlit {
namespace js {
namespace object {

class JSOrdinaryObject : public JSObject {
  protected:
    /* TODO: Use LinkedHashMap instead */
    util::ArrayList<JSPropertyKey>* propKey = nullptr;
    util::ArrayList<Property>* propVal = nullptr;

    JSObject* prototype_ = nullptr;
    bool extensible = true;

    int LookupPropertyId(const gc::Handle<JSPropertyKey>&);
    gc::Handle<Property> LookupProperty(const gc::Handle<JSPropertyKey>&);

  public:
    JSOrdinaryObject(const gc::Handle<JSObject>&);

  public:
    /* 7.3.4 CreateDataProperty */
    static bool CreateDataProperty(const gc::Handle<JSObject>&, const gc::Handle<JSPropertyKey>&, const gc::Handle<JSValue>&);

    static bool ValidateAndApplyPropertyDescriptor(
        const gc::Handle<JSOrdinaryObject>&,
        const gc::Handle<JSPropertyKey>&,
        bool extensible,
        const PropertyDescriptor&,
        const util::Optional<PropertyDescriptor>&
    );
    static bool OrdinaryHasProperty(const gc::Handle<JSOrdinaryObject>&, const gc::Handle<JSPropertyKey>&);

    static util::Optional<PropertyDescriptor> OrdinaryGetOwnProperty(
        const gc::Handle<JSOrdinaryObject>&, const gc::Handle<JSPropertyKey>&);
    static bool OrdinaryDefineOwnProperty(const gc::Handle<JSOrdinaryObject>&, const gc::Handle<JSPropertyKey>&, const PropertyDescriptor&);

    virtual gc::Handle<JSObject> GetPrototypeOf() override final;
    virtual bool SetPrototypeOf(const gc::Handle<JSObject>&) override final;
    virtual bool IsExtensible() override final;
    virtual bool PreventExtensions() override final;
    virtual util::Optional<PropertyDescriptor> GetOwnProperty(const gc::Handle<JSPropertyKey>&) override;
    virtual bool DefineOwnProperty(const gc::Handle<JSPropertyKey>&, const PropertyDescriptor&) override;
    virtual bool HasProperty(const gc::Handle<JSPropertyKey>&) override;
    virtual gc::Handle<JSValue> Get(const gc::Handle<JSPropertyKey>&, const gc::Handle<JSValue>&) override;
    virtual bool Set(const gc::Handle<JSPropertyKey>&, const gc::Handle<JSValue>&, const gc::Handle<JSValue>&) override;
    virtual bool Delete(const gc::Handle<JSPropertyKey>&) override;

    virtual gc::Handle<gc::Array<JSPropertyKey>> OwnPropertyKeys() override;

    virtual void IterateField(const gc::FieldIterator& callback) override;
};

}
}
}

#endif