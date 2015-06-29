#include "../all.h"

#include "Builtin.h"

#include "../vm/Context.h"
#include "../object/Exotics.h"

#include "../../util/ScopeExit.h"

#include <cstdio>

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::builtin;

namespace {
Handle<GeneratorObject> GeneratorValidate(const Handle<JSValue>& val) {
    if (!Testing::Is<JSObject>(val)) {
        Exceptions::ThrowIncompatibleReceiverTypeError("GeneratorValidate");
    }
    Handle<GeneratorObject> generator = val.ExactCheckedCastTo<GeneratorObject>();
    if (!generator) {
        Exceptions::ThrowIncompatibleReceiverTypeError("GeneratorValidate");
    }
    if (generator->generatorState() == GeneratorObject::GeneratorState::kExecuting) {
        Exceptions::ThrowTypeError("Resume an executing generator is invalid");
    }
    return generator;
}
}

Handle<JSValue> Generator::prototype::next(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    Handle<GeneratorObject> gen = GeneratorValidate(that);
    if (gen->generatorState() == GeneratorObject::GeneratorState::kCompleted) {
        return Iterators::CreateIterResultObject(nullptr, true);
    }
    Handle<JSValue> value = GetArg(args, 0);
    Handle<BytecodeContext> ctx = gen->generatorContext();
    gen->generatorState(GeneratorObject::GeneratorState::kExecuting);
    Context::PushContext(ctx);
    NORLIT_SCOPE_EXIT{ Context::PopContext(); };
    ctx->Push(value);
    BytecodeContext::ReturnStatus status = ctx->Run();
    if (status == BytecodeContext::ReturnStatus::kReturn) {
        gen->generatorState(GeneratorObject::GeneratorState::kCompleted);
        return Iterators::CreateIterResultObject(ctx->Pop(), true);
    } else {
        gen->generatorState(GeneratorObject::GeneratorState::kSuspendedYield);
        return Iterators::CreateIterResultObject(ctx->Pop(), false);
    }
}

Handle<JSValue> Generator::prototype::return_(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    Handle<GeneratorObject> gen = GeneratorValidate(that);
    Handle<JSValue> value = GetArg(args, 0);
    if (gen->generatorState() == GeneratorObject::GeneratorState::kSuspendedStart) {
        gen->generatorState(GeneratorObject::GeneratorState::kCompleted);
    }
    if (gen->generatorState() == GeneratorObject::GeneratorState::kCompleted) {
        return Iterators::CreateIterResultObject(value, true);
    }

    printf("Warning: Generator.prototype.return is currently not standard compilant. Finally block will not be executed in any case.\n");
    gen->generatorState(GeneratorObject::GeneratorState::kCompleted);
    return Iterators::CreateIterResultObject(value, true);
}


Handle<JSValue> Generator::prototype::throw_(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    Handle<GeneratorObject> gen = GeneratorValidate(that);
    Handle<JSValue> exception = GetArg(args, 0);
    if (gen->generatorState() == GeneratorObject::GeneratorState::kSuspendedStart) {
        gen->generatorState(GeneratorObject::GeneratorState::kCompleted);
    }
    if (gen->generatorState() == GeneratorObject::GeneratorState::kCompleted) {
        throw ESException(exception);
    }
    Handle<BytecodeContext> ctx = gen->generatorContext();
    gen->generatorState(GeneratorObject::GeneratorState::kExecuting);
    Context::PushContext(ctx);
    NORLIT_SCOPE_EXIT{ Context::PopContext(); };
    if (!ctx->HandleException(exception)) {
        throw ESException(exception);
    }
    BytecodeContext::ReturnStatus status = ctx->Run();
    if (status == BytecodeContext::ReturnStatus::kReturn) {
        gen->generatorState(GeneratorObject::GeneratorState::kCompleted);
        return Iterators::CreateIterResultObject(ctx->Pop(), true);
    } else {
        gen->generatorState(GeneratorObject::GeneratorState::kSuspendedYield);
        return Iterators::CreateIterResultObject(ctx->Pop(), false);
    }
}
