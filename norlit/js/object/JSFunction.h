#ifndef NORLIT_JS_OBJECT_FUNCTION_H
#define NORLIT_JS_OBJECT_FUNCTION_H

#include "../common.h"

#include "JSOrdinaryObject.h"

namespace norlit {
namespace js {

namespace vm {
class Realm;
class Environment;
}

namespace bytecode {
class Code;
}

enum class FunctionKind : uint8_t;

    namespace object {

class ESFunctionBase : public JSOrdinaryObject {
  protected:
    vm::Realm* realm_ = nullptr;

  public:
    ESFunctionBase(const gc::Handle<JSObject>&, const gc::Handle<vm::Realm>&);

    gc::Handle<vm::Realm> realm() {
        return realm_;
    }

    virtual bool IsCallable() override;
    virtual bool IsConstructor() override;
    virtual void IterateField(const gc::FieldIterator&) override;
};

class ESFunction : public ESFunctionBase {
  public:
    enum class ConstructorKind {
        kBase,
        kDerived,
        kNonConstructor
    };
    enum class ThisMode {
        kLexical,
        kStrict,
        kGlobal
    };

    NORLIT_DEFINE_FIELD(vm::Environment, environment)
    NORLIT_DEFINE_FIELD(JSObject, homeObject)
    NORLIT_DEFINE_FIELD(bytecode::Code, code)

    NORLIT_DEFINE_FIELD_POD(FunctionKind, functionKind)
    NORLIT_DEFINE_FIELD_POD(ConstructorKind, constructorKind)
    NORLIT_DEFINE_FIELD_POD(ThisMode, thisMode);
    NORLIT_DEFINE_FIELD_POD(bool, strict)

  public:
    ESFunction(const gc::Handle<JSObject>& proto, const gc::Handle<vm::Realm>& realm) :ESFunctionBase(proto, realm) {}

    virtual gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&) override;
    virtual gc::Handle<JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<JSObject>&) override;
    virtual void IterateField(const gc::FieldIterator&) override;
};

class ESNativeFunction : public ESFunctionBase {
  public:
    using CallFunc = gc::Handle<JSValue>(*)(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    using CtorFunc = gc::Handle<JSObject>(*)(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<JSObject>&);
  private:
    CallFunc call;
    CtorFunc ctor;
  public:
    ESNativeFunction(const gc::Handle<JSObject>&, const gc::Handle<vm::Realm>&, CallFunc, CtorFunc);

    virtual gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&) override;
    virtual gc::Handle<JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<JSObject>&) override;
};

class BoundFunction final : public JSOrdinaryObject {
    JSObject* boundFunction = nullptr;
    JSValue* boundThis = nullptr;
    gc::Array<JSValue>* boundArguments = nullptr;
    bool canConstruct = false;

    BoundFunction(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}

  public:
    static gc::Handle<JSObject> New(const gc::Handle<JSObject>&, const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);

    gc::Handle<JSObject> boundTargetFunction() const {
        return boundFunction;
    }

    virtual bool IsCallable() override;
    virtual bool IsConstructor() override;
    virtual gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&) override;
    virtual gc::Handle<JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<JSObject>&) override;

    virtual void IterateField(const gc::FieldIterator&) override;
};


}
}
}

#endif