#include "../all.h"

#include "JSFunction.h"
#include "Exotics.h"

#include "../vm/Realm.h"
#include "../vm/Context.h"

#include "../../gc/Heap.h"

#include "../../util/Arrays.h"
#include "../../util/ScopeExit.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::util;

ESFunctionBase::ESFunctionBase(const Handle<JSObject>& proto, const Handle<vm::Realm>& realm):JSOrdinaryObject(proto) {
    this->WriteBarrier(&this->realm_, realm);
}

bool ESFunctionBase::IsCallable() {
    return true;
}

bool ESFunctionBase::IsConstructor() {
    return true;
}

void ESFunctionBase::IterateField(const FieldIterator& iter) {
    JSOrdinaryObject::IterateField(iter);
    iter(&this->realm_);
}

void ESFunction::IterateField(const FieldIterator& iter) {
    ESFunctionBase::IterateField(iter);
    iter(&this->environment_);
    iter(&this->homeObject_);
    iter(&this->code_);
}


Handle<JSValue> ESFunction::Call(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    Handle<ESFunction> self = this;
    if (self->functionKind_ == FunctionKind::kClassConstructor) {
        Exceptions::ThrowTypeError("Class constructors cannot be invoked without 'new'");
    }

    Handle<FunctionEnvironment> funcEnv = new FunctionEnvironment(self, nullptr);
    Handle<BytecodeContext> ctx = new BytecodeContext(funcEnv, funcEnv, self->realm_, self, self->code_);
    if (self->thisMode() != ThisMode::kLexical) {
        if (self->thisMode() == ThisMode::kStrict) {
            funcEnv->BindThisValue(that);
        } else {
            if (!that || Testing::Is<JSNull>(that)) {
                throw "Bind global";
            } else {
                Handle<JSObject> thisValue = Conversion::ToObject(that);
                funcEnv->BindThisValue(thisValue);
            }
        }
    }
    Context::PushContext(ctx);
    NORLIT_SCOPE_EXIT{ Context::PopContext(); };
    Handle<JSObject> arguments = Objects::CreateArrayFromList(args);
    ctx->Push(arguments);
    if (self->functionKind() == FunctionKind::kGenerator) {
        Handle<GeneratorObject> G = Objects::OrdinaryCreateFromConstructor<GeneratorObject>(self, &Realm::GeneratorPrototype);
        G->generatorContext(ctx);
        G->generatorState(GeneratorObject::GeneratorState::kSuspendedStart);
        return G;
    } else {
        ctx->Run();
        return ctx->Pop();
    }
}

Handle<JSObject> ESFunction::Construct(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {
    Handle<ESFunction> self = this;

    if (self->constructorKind() == ConstructorKind::kNonConstructor) {
        throw "internal error: non-constructor get called";
    }

    Handle<JSObject> thisArgument;
    if (self->constructorKind() == ConstructorKind::kBase) {
        thisArgument = Objects::OrdinaryCreateFromConstructor(target, &Realm::ObjectPrototype);
    }

    Handle<FunctionEnvironment> funcEnv = new FunctionEnvironment(self, nullptr);
    Handle<BytecodeContext> ctx = new BytecodeContext(funcEnv, funcEnv, self->realm_, self, self->code_);

    if (self->constructorKind() == ConstructorKind::kBase) {
        funcEnv->BindThisValue(thisArgument);
    }


    {
        Context::PushContext(ctx);
        NORLIT_SCOPE_EXIT{ Context::PopContext(); };
        Handle<JSObject> arguments = Objects::CreateArrayFromList(args);
        ctx->Push(arguments);
        if (self->functionKind() == FunctionKind::kGenerator) {
            Handle<GeneratorObject> G = Objects::OrdinaryCreateFromConstructor<GeneratorObject>(self, &Realm::GeneratorPrototype);
            G->generatorContext(ctx);
            G->generatorState(GeneratorObject::GeneratorState::kSuspendedStart);
            return G;
        }
        ctx->Run();
    }

    Handle<JSValue> result = ctx->Pop();

    if (Testing::Is<JSObject>(result)) {
        return result.CastTo<JSObject>();
    }
    if (self->constructorKind() == ConstructorKind::kBase) {
        return thisArgument;
    }
    if (result) {
        Exceptions::ThrowTypeError("Constructor must return object");
    }
    result = funcEnv->GetThisBinding();
    if (!Testing::Is<JSObject>(result)) {
        throw "internal error: this binding is not a object";
    }
    return result.CastTo<JSObject>();
}

ESNativeFunction::ESNativeFunction(const Handle<JSObject>& proto, const Handle<Realm>& realm, CallFunc func, CtorFunc ctor):ESFunctionBase(proto, realm) {
    this->call = func;
    this->ctor = ctor;
}

Handle<JSValue> ESNativeFunction::Call(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    Handle<ESNativeFunction> self = this;
    if (self->call) {
        Handle<Context> ctx = new Context(self->realm_, self, that);
        Context::PushContext(ctx);
        NORLIT_SCOPE_EXIT { Context::PopContext(); };
        Handle<JSValue> result = call(that, args);
        return result;
    } else {
        throw "Call not implemented";
    }
}

Handle<JSObject> ESNativeFunction::Construct(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {
    Handle<ESNativeFunction> self = this;
    if (self->ctor) {
        Handle<Context> ctx = new Context(self->realm_, self, nullptr);
        Context::PushContext(ctx);
        NORLIT_SCOPE_EXIT{ Context::PopContext(); };
        Handle<JSObject> result = ctor(args, target);
        return result;
    } else {
        throw "Call not implemented";
    }
}

Handle<JSObject> BoundFunction::New(const Handle<JSObject>& targetFunction, const Handle<JSValue>& boundThis, const Handle<Array<JSValue>>& boundArguments) {
    Handle<JSObject> proto = targetFunction->GetPrototypeOf();
    Handle<BoundFunction> obj = new BoundFunction(proto);
    if (targetFunction->IsConstructor()) {
        obj->canConstruct = true;
    }
    obj->WriteBarrier(&obj->boundFunction, targetFunction);
    obj->WriteBarrier(&obj->boundThis, boundThis);
    obj->WriteBarrier(&obj->boundArguments, boundArguments);
    return obj;
}

bool BoundFunction::IsCallable() {
    return true;
}

bool BoundFunction::IsConstructor() {
    return canConstruct;
}

Handle<JSValue> BoundFunction::Call(const Handle<JSValue>& that, const Handle<Array<JSValue>>& argumentsList) {
    Handle<JSObject> target = this->boundFunction;
    Handle<JSValue> boundThis = this->boundThis;
    Handle<Array<JSValue>> args = Arrays::Concat<JSValue>(this->boundArguments, argumentsList);
    return Objects::Call(target, boundThis, args);
}

Handle<JSObject> BoundFunction::Construct(const Handle<Array<JSValue>>& argumentsList, const Handle<JSObject>& originalNewTarget) {
    Handle<JSObject> target = this->boundFunction;
    Handle<JSObject> newTarget = originalNewTarget == this ? target : originalNewTarget;
    Handle<Array<JSValue>> args = Arrays::Concat<JSValue>(this->boundArguments, argumentsList);
    return Objects::Construct(target, args, newTarget);
}

void BoundFunction::IterateField(const FieldIterator& iter) {
    JSOrdinaryObject::IterateField(iter);
    iter(&this->boundFunction);
    iter(&this->boundThis);
    iter(&this->boundArguments);
}