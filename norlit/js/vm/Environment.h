#ifndef NORLIT_JS_VM_ENVREC_H
#define NORLIT_JS_VM_ENVREC_H

#include "../common.h"
#include "../JSValue.h"
#include "../../util/HashMap.h"

namespace norlit {
namespace js {

class JSString;

namespace object {
class JSObject;
class ESFunction;
}

namespace vm {

class Environment : public gc::Object {
    Environment* outer_ = nullptr;
  public:
    Environment(const gc::Handle<Environment>&);

    gc::Handle<Environment> outer() {
        return outer_;
    }

    virtual bool HasBinding(const gc::Handle<JSString>&) = 0;
    virtual void CreateMutableBinding(const gc::Handle<JSString>&, bool = false) = 0;
    virtual void CreateImmutableBinding(const gc::Handle<JSString>&, bool = false) = 0;
    virtual void InitializeBinding(const gc::Handle<JSString>&, const gc::Handle<JSValue>&) = 0;
    virtual void SetMutableBinding(const gc::Handle<JSString>&, const gc::Handle<JSValue>&, bool) = 0;
    virtual gc::Handle<JSValue> GetBindingValue(const gc::Handle<JSString>&, bool) = 0;
    virtual bool DeleteBinding(const gc::Handle<JSString>&) = 0;
    virtual bool HasThisBinding() = 0;
    virtual bool HasSuperBinding() = 0;
    virtual gc::Handle<object::JSObject> WithBaseObject() = 0;

    virtual gc::Handle<JSValue> GetThisBinding();

    virtual void IterateField(const gc::FieldIterator& iter) override;
};

class DeclarativeEnvironemnt :public Environment {
    struct Entry;

    util::HashMap<JSString, Entry>* entries = nullptr;
  public:
    DeclarativeEnvironemnt(const gc::Handle<Environment>&);

    virtual bool HasBinding(const gc::Handle<JSString>&);
    virtual void CreateMutableBinding(const gc::Handle<JSString>&, bool = false);
    virtual void CreateImmutableBinding(const gc::Handle<JSString>&, bool = false);
    virtual void InitializeBinding(const gc::Handle<JSString>&, const gc::Handle<JSValue>&);
    virtual void SetMutableBinding(const gc::Handle<JSString>&, const gc::Handle<JSValue>&, bool);
    virtual gc::Handle<JSValue> GetBindingValue(const gc::Handle<JSString>&, bool);
    virtual bool DeleteBinding(const gc::Handle<JSString>&);
    virtual bool HasThisBinding();
    virtual bool HasSuperBinding();
    virtual gc::Handle<object::JSObject> WithBaseObject();

    virtual void IterateField(const gc::FieldIterator& iter) override;
};

class ObjectEnvironment final :public Environment {
    object::JSObject* bindingObject_ = nullptr;
    bool with;
  public:
    ObjectEnvironment(const gc::Handle<object::JSObject>&, const gc::Handle<Environment>&, bool with = false);

    virtual bool HasBinding(const gc::Handle<JSString>&);
    virtual void CreateMutableBinding(const gc::Handle<JSString>&, bool = false);
    virtual void CreateImmutableBinding(const gc::Handle<JSString>&, bool = false);
    virtual void InitializeBinding(const gc::Handle<JSString>&, const gc::Handle<JSValue>&);
    virtual void SetMutableBinding(const gc::Handle<JSString>&, const gc::Handle<JSValue>&, bool);
    virtual gc::Handle<JSValue> GetBindingValue(const gc::Handle<JSString>&, bool);
    virtual bool DeleteBinding(const gc::Handle<JSString>&);
    virtual bool HasThisBinding();
    virtual bool HasSuperBinding();
    virtual gc::Handle<object::JSObject> WithBaseObject();

    virtual void IterateField(const gc::FieldIterator& iter) override;

    friend class GlobalEnvironment;
};

class FunctionEnvironment final :public DeclarativeEnvironemnt {
  public:
    enum class ThisBindingStatus {
        kLexical,
        kUninitialized,
        kInitialized
    };

    NORLIT_DEFINE_FIELD(JSValue, thisValue);
    NORLIT_DEFINE_FIELD(object::ESFunction, functionObject);
    NORLIT_DEFINE_FIELD(object::JSObject, homeObject);
    NORLIT_DEFINE_FIELD(object::JSObject, newTarget);
    NORLIT_DEFINE_FIELD_POD(ThisBindingStatus, thisBindingStatus);
  public:
    FunctionEnvironment(const gc::Handle<object::ESFunction>&, const gc::Handle<object::JSObject>&);

    void BindThisValue(const gc::Handle<JSValue>&);
    virtual bool HasThisBinding() override;
    virtual bool HasSuperBinding() override;
    virtual gc::Handle<JSValue> GetThisBinding() override;

    virtual void IterateField(const gc::FieldIterator& iter) override;
};

class GlobalEnvironment final :public Environment {
    DeclarativeEnvironemnt* declRecord_ = nullptr;
    ObjectEnvironment* objectRecord_ = nullptr;
  public:
    GlobalEnvironment(const gc::Handle<object::JSObject>&);

    virtual bool HasBinding(const gc::Handle<JSString>&) override;
    virtual void CreateMutableBinding(const gc::Handle<JSString>&, bool = false) override;
    virtual void CreateImmutableBinding(const gc::Handle<JSString>&, bool = false) override;
    virtual void InitializeBinding(const gc::Handle<JSString>&, const gc::Handle<JSValue>&) override;
    virtual void SetMutableBinding(const gc::Handle<JSString>&, const gc::Handle<JSValue>&, bool) override;
    virtual gc::Handle<JSValue> GetBindingValue(const gc::Handle<JSString>&, bool) override;
    virtual bool DeleteBinding(const gc::Handle<JSString>&) override;
    virtual bool HasThisBinding() override;
    virtual bool HasSuperBinding() override;
    virtual gc::Handle<object::JSObject> WithBaseObject() override;
    virtual gc::Handle<JSValue> GetThisBinding() override;

    virtual void IterateField(const gc::FieldIterator& iter) override;
};

}
}
}

#endif