#ifndef NORLIT_JS_VM_CONTEXT_H
#define NORLIT_JS_VM_CONTEXT_H

#include "../../gc/Object.h"
#include "../../util/ArrayList.h"

#include "../JSValue.h"
#include "../object/JSFunction.h"

namespace norlit {
namespace js {

namespace bytecode {
class Code;
}

namespace vm {

class Environment;
class Realm;

class Context : public gc::Object {
    Realm* realm = nullptr;
    JSValue* that = nullptr;
    object::ESFunctionBase* func = nullptr;
  public:
    static util::ArrayList<Context>& GetContextStack();
    static void PushContext(const gc::Handle<Context>&);
    static void PopContext();
    static gc::Handle<Context> CurrentContext();
    static gc::Handle<Realm> CurrentRealm();

    gc::Handle<object::ESFunctionBase> function() {
        return func;
    }
    gc::Handle<JSValue> thisPointer() {
        return that;
    }

    Context(const gc::Handle<Realm>&, const gc::Handle<object::ESFunctionBase>&, const gc::Handle<JSValue>&);
    virtual void IterateField(const gc::FieldIterator&) override;
};

class BytecodeContext : public Context {
    util::ArrayList<JSValue>* stack = nullptr;

    Environment* lexEnv = nullptr;
    Environment* varEnv = nullptr;

    bytecode::Code* code = nullptr;
    size_t ip = 0;

    template<typename T>
    gc::Handle<T> PopAs();

    template<typename T>
    gc::Handle<T> GetConstantAs(uint16_t);

    gc::Handle<Environment> GetThisEnvironment();
    gc::Handle<JSValue> ResolveThisBinding();

    uint16_t Fetch16();

    virtual void IterateField(const gc::FieldIterator&) override final;
  public:
    enum class ReturnStatus {
        kNormal,
        kReturn,
        kYield
    };

    BytecodeContext(
        const gc::Handle<Environment>& lexEnv,
        const gc::Handle<Environment>& varEnv,
        const gc::Handle<Realm>& realm,
        const gc::Handle<object::ESFunctionBase>& func,
        const gc::Handle<bytecode::Code>& code);

    void Push(const gc::Handle<JSValue>&);
    gc::Handle<JSValue> Pop();
    gc::Handle<JSValue> Peek();


    bool HandleException(const gc::Handle<JSValue>&);

    ReturnStatus Step();
    ReturnStatus Run();
};

}
}
}

#endif