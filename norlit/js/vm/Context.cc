#include "../all.h"

#include "Context.h"
#include "../bytecode/Code.h"
#include "../bytecode/Instruction.h"
#include "../../gc/Heap.h"

#include "../object/JSFunction.h"

#include "../../util/ScopeExit.h"

#include <cstdio>

#include "Environment.h"
#include "Realm.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::bytecode;
using namespace norlit::util;

namespace {

Handle<JSObject> GetPlaceholder() {
    static Handle<JSObject> placeholder = new JSOrdinaryObject(nullptr);
    return placeholder;
}

Handle<JSObject> GetElisionPlaceholder() {
    static Handle<JSObject> placeholder = new JSOrdinaryObject(nullptr);
    return placeholder;
}

}

ArrayList<Context>& Context::GetContextStack() {
    static ArrayList<Context> stack;
    return stack;
}

void Context::PushContext(const Handle<Context>& ctx) {
    static size_t stackLimit = 128;
    ArrayList<Context>& stack = GetContextStack();
    if (stack.Size() >= stackLimit) {
        if (stackLimit == 128) {
            stackLimit += 16;
            NORLIT_SCOPE_EXIT {stackLimit -= 16;};
            Exceptions::ThrowRangeError("Stack overflow");
        } else {
            throw ESException(JSString::New("Stack overflow"));
        }
    }
    stack.Add(ctx);
}

void Context::PopContext() {
    GetContextStack().RemoveLast();
}

Handle<Context> Context::CurrentContext() {
    return GetContextStack().GetLast();
}

Handle<Realm> Context::CurrentRealm() {
    return GetContextStack().GetLast()->realm;
}

Context::Context(const Handle<Realm>& realm, const Handle<ESFunctionBase>& func, const Handle<JSValue>& that) {
    this->WriteBarrier(&this->realm, realm);
    this->WriteBarrier(&this->func, func);
    this->WriteBarrier(&this->that, that);
}

void Context::IterateField(const FieldIterator& iter) {
    iter(&this->realm);
    iter(&this->func);
    iter(&this->that);
}


BytecodeContext::BytecodeContext(
    const Handle<Environment>& lexEnv,
    const Handle<Environment>& varEnv,
    const Handle<Realm>& realm,
    const Handle<ESFunctionBase>& func,
    const Handle<bytecode::Code>& code):Context(realm, func, nullptr) {
    NoGC _;
    WriteBarrier(&this->stack, new ArrayList<JSValue>());
    WriteBarrier(&this->lexEnv, lexEnv);
    WriteBarrier(&this->varEnv, varEnv);
    WriteBarrier(&this->code, code);
}

void BytecodeContext::Push(const Handle<JSValue>& v) {
    stack->Add(v);
}

Handle<JSValue> BytecodeContext::Pop() {
    Handle<JSValue> val= stack->Remove(stack->Size() - 1);
    return val;
}

Handle<JSValue> BytecodeContext::Peek() {
    return stack->Get(stack->Size() - 1);
}

template<typename T>
Handle<T> BytecodeContext::PopAs() {
    Handle<JSValue> val = this->Pop();
    assert(Testing::Is<T>(val));
    return val.CastTo<T>();
}


template<typename T>
Handle<T> BytecodeContext::GetConstantAs(uint16_t index) {
    Handle<JSValue> val = this->code->GetConstant(index);
    assert(Testing::Is<T>(val));
    return val.CastTo<T>();
}

Handle<Environment> BytecodeContext::GetThisEnvironment() {
    Handle<Environment> lex = this->lexEnv;
    while (true) {
        if (lex->HasThisBinding()) {
            return lex;
        }
        lex = lex->outer();
    }
}

Handle<JSValue> BytecodeContext::ResolveThisBinding() {
    return this->GetThisEnvironment()->GetThisBinding();
}

void BytecodeContext::IterateField(const FieldIterator& iter) {
    Context::IterateField(iter);
    iter(&this->stack);
    iter(&this->lexEnv);
    iter(&this->varEnv);
    iter(&this->code);
}

uint16_t BytecodeContext::Fetch16() {
    uint8_t hi = this->code->At(this->ip++);
    uint8_t lo = this->code->At(this->ip++);
    return (hi << 8) | lo;
}

BytecodeContext::ReturnStatus BytecodeContext::Step() {
    Handle<BytecodeContext> self = this;
    Instruction ins = static_cast<Instruction>(self->code->At(self->ip++));
    Handle<JSValue> result;

    switch (ins) {
        case Instruction::kOne:
            self->Push(JSNumber::One());
            break;
        case Instruction::kUndef:
            self->Push(nullptr);
            break;
        case Instruction::kTrue:
            self->Push(JSBoolean::New(true));
            break;
        case Instruction::kLoad: {
            uint16_t index = self->Fetch16();
            Handle<JSValue> tmp = self->code->GetConstant(index);
            self->Push(tmp);
            break;
        }

        case Instruction::kJump: {
            ip = self->Fetch16();
            break;
        }
        case Instruction::kJumpIfTrue: {
            uint16_t target = self->Fetch16();
            if (self->PopAs<JSBoolean>()->Value()) {
                ip = target;
            }
            break;
        }

        case Instruction::kFunction: {
            uint16_t codeIndex = self->Fetch16();
            Handle<Code> code = self->code->GetCode(codeIndex);
            Handle<JSObject> F = Objects::FunctionCreate(FunctionKind::kNormal, code, self->lexEnv, true);
            Objects::MakeConstructor(F);
            self->Push(F);
            break;
        }

        case Instruction::kGenerator: {
            uint16_t codeIndex = self->Fetch16();
            Handle<Code> code = self->code->GetCode(codeIndex);
            Handle<JSObject> F = Objects::GeneratorFunctionCreate(FunctionKind::kNormal, code, self->lexEnv, true);
            Handle<JSObject> prototype = Objects::ObjectCreate(CurrentRealm()->GeneratorPrototype());
            Objects::MakeConstructor(F, true, prototype);
            self->Push(F);
            break;
        }

        case Instruction::kInstanceOf: {
            Handle<JSValue> right = self->Pop();
            Handle<JSValue> left = self->Pop();
            result = JSBoolean::New(Objects::InstanceOfOperator(left, right));
            self->Push(result);
            break;
        }

        case Instruction::kTypeOf: {
            Handle<JSValue> operand = self->Pop();
            switch (operand->GetType()) {
                case JSValue::Type::kUndefined:
                    result = JSString::New("undefined");
                    break;
                case JSValue::Type::kNull:
                    result = JSString::New("null");
                    break;
                case JSValue::Type::kNumber:
                    result = JSString::New("number");
                    break;
                case JSValue::Type::kString:
                    result = JSString::New("string");
                    break;
                case JSValue::Type::kBoolean:
                    result = JSString::New("boolean");
                    break;
                case JSValue::Type::kSymbol:
                    result = JSString::New("symbol");
                    break;
                default:
                    if (operand.CastTo<JSObject>()->IsCallable()) {
                        result = JSString::New("function");
                    } else {
                        result = JSString::New("object");
                    }
                    break;
            }
            self->Push(result);
            break;
        }

        case Instruction::kDefVar: {
            uint16_t index = self->Fetch16();
            Handle<JSValue> tmp = self->code->GetConstant(index);
            assert(tmp->GetType() == JSValue::Type::kString);
            Handle<JSString> name = tmp.CastTo<JSString>();

            if (self->lexEnv->HasBinding(name)) {
                break;
            }

            self->lexEnv->CreateMutableBinding(name);
            self->lexEnv->InitializeBinding(name, nullptr);
            break;
        }

        case Instruction::kDefLet: {
            uint16_t index = self->Fetch16();
            Handle<JSValue> tmp = self->code->GetConstant(index);
            assert(tmp->GetType() == JSValue::Type::kString);
            Handle<JSString> name = tmp.CastTo<JSString>();

            self->lexEnv->CreateMutableBinding(name);
            break;
        }

        case Instruction::kDefConst: {
            uint16_t index = self->Fetch16();
            Handle<JSValue> tmp = self->code->GetConstant(index);
            assert(tmp->GetType() == JSValue::Type::kString);
            Handle<JSString> name = tmp.CastTo<JSString>();

            self->lexEnv->CreateImmutableBinding(name);
            break;
        }

        case Instruction::kInitDef: {
            uint16_t index = self->Fetch16();
            Handle<JSValue> tmp = self->code->GetConstant(index);
            assert(tmp->GetType() == JSValue::Type::kString);
            Handle<JSString> name = tmp.CastTo<JSString>();

            Handle<JSValue> val = self->Pop();

            self->lexEnv->InitializeBinding(name, val);
            break;
        }

        case Instruction::kPushScope: {
            Handle<Environment> newEnv = new DeclarativeEnvironemnt(self->lexEnv);
            self->lexEnv = newEnv;
            break;
        }

        case Instruction::kPopScope: {
            self->lexEnv = self->lexEnv->outer();
            break;
        }

        case Instruction::kGetName: {
            uint16_t index = self->Fetch16();
            Handle<JSValue> tmp = self->code->GetConstant(index);
            assert(tmp->GetType() == JSValue::Type::kString);
            Handle<JSString> name = tmp.CastTo<JSString>();

            Handle<Environment> lex = self->lexEnv;
            while (lex) {
                if (lex->HasBinding(name)) {
                    break;
                }
                lex = lex->outer();
            }

            if (lex) {
                result = lex->GetBindingValue(name, false);
                self->Push(result);
            } else {
                Exceptions::ThrowReferenceError(name);
            }
            break;
        }


        case Instruction::kGetNameOrUndef: {
            uint16_t index = self->Fetch16();
            Handle<JSValue> tmp = self->code->GetConstant(index);
            assert(tmp->GetType() == JSValue::Type::kString);
            Handle<JSString> name = tmp.CastTo<JSString>();

            Handle<Environment> lex = self->lexEnv;
            while (lex) {
                if (lex->HasBinding(name)) {
                    break;
                }
                lex = lex->outer();
            }

            if (lex) {
                result = lex->GetBindingValue(name, false);
                self->Push(result);
            } else {
                self->Push(nullptr);
            }
            break;
        }

        case Instruction::kPutName: {
            uint16_t index = self->Fetch16();
            Handle<JSValue> value = self->Peek();
            Handle<JSString> name = self->GetConstantAs<JSString>(index);

            Handle<Environment> lex = self->lexEnv;
            while (lex) {
                if (lex->HasBinding(name)) {
                    break;
                }
                lex = lex->outer();
            }

            if (lex) {
                lex->SetMutableBinding(name, value, true);
            } else {
                Exceptions::ThrowReferenceError(name);
                throw "TODO: GetGlobalObject";
                // throw "ReferenceError if strict";
            }
            break;
        }

        case Instruction::kDeleteName: {
            uint16_t index = self->Fetch16();
            Handle<JSValue> value = self->Peek();
            Handle<JSString> name = self->GetConstantAs<JSString>(index);

            Handle<Environment> lex = self->lexEnv;
            while (lex) {
                if (lex->HasBinding(name)) {
                    break;
                }
                lex = lex->outer();
            }

            if (lex) {
                result = JSBoolean::New(lex->DeleteBinding(name));
            } else {
                result = JSBoolean::New(true);
            }

            self->Push(result);

            break;
        }

        case Instruction::kGetProperty: {
            Handle<JSValue> propAsValue = self->Pop();
            Handle<JSValue> base = self->Pop();

            Testing::RequireObjectCoercible(base);
            Handle<JSPropertyKey> prop = Conversion::ToPropertyKey(propAsValue);

            result = Objects::GetV(base, prop);
            self->Push(result);
            break;
        }

        case Instruction::kGetPropertyNoPop: {
            Handle<JSValue> propAsValue = self->Pop();
            Handle<JSValue> base = self->Peek();

            Testing::RequireObjectCoercible(base);
            Handle<JSPropertyKey> prop = Conversion::ToPropertyKey(propAsValue);

            result = Objects::GetV(base, prop);

            self->Push(prop);
            self->Push(result);
            break;
        }

        case Instruction::kSetProperty: {
            Handle<JSValue> val = self->Pop();
            Handle<JSValue> propAsValue = self->Pop();
            Handle<JSValue> base = self->Pop();

            Testing::RequireObjectCoercible(base);
            Handle<JSPropertyKey> prop = Conversion::ToPropertyKey(propAsValue);

            switch (base->GetType()) {
                case JSValue::Type::kObject:
                    Objects::Set(base.CastTo<JSObject>(), prop, val, true);
                    break;
                default:
                    Objects::Set(Conversion::ToObject(base), prop, val, true);
                    // throw "TODO SetV";
                    break;
            }
            self->Push(val);
            break;
        }

        case Instruction::kDeleteProperty: {
            Handle<JSValue> propAsValue = self->Pop();
            Handle<JSValue> base = self->Pop();

            Testing::RequireObjectCoercible(base);
            Handle<JSPropertyKey> prop = Conversion::ToPropertyKey(propAsValue);

            bool result;

            switch (base->GetType()) {
                case JSValue::Type::kObject:
                    result = base.CastTo<JSObject>()->Delete(prop);
                    break;
                case JSValue::Type::kString: {
                    if (Handle<JSString> keyAsStr = Testing::CastIf<JSString>(prop)) {
                        Handle<JSString> self = base.CastTo<JSString>();
                        int64_t index = Conversion::ToIntegerIndex(keyAsStr);
                        if (index != -1 && index < static_cast<int64_t>(self->Length())) {
                            result = true;
                            break;
                        }
                        if (keyAsStr == JSString::New("length")) {
                            result = true;
                            break;
                        }
                    }
                    result = false;
                    break;
                }
                default:
                    result = false;
                    break;
            }

            self->Push(JSBoolean::New(result));
            break;
        }

        case Instruction::kCreateDataProperty: {
            Handle<JSValue> val = self->Pop();
            Handle<JSValue> propAsValue = self->Pop();
            Handle<JSObject> base = self->PopAs<JSObject>();
            Handle<JSPropertyKey> prop = Conversion::ToPropertyKey(propAsValue);
            Objects::CreateDataPropertyOrThrow(base, prop, val);
            self->Push(base);
            break;
        }

        case Instruction::kCreateObject: {
            Handle<JSObject> obj = Objects::ObjectCreate(CurrentRealm()->ObjectPrototype());
            self->Push(obj);
            break;
        }

        case Instruction::kCall: {
            Handle<Array<JSValue>> args;
            {
                Handle<JSValue> guard = GetPlaceholder();
                size_t size = self->stack->Size(), i;
                for (i = size - 1;; i--) {
                    if (self->stack->Get(i) == guard) {
                        break;
                    } else if (i == 0) {
                        assert(!"No array start placeholder found");
                    }
                }
                size_t argCount = size - i - 1;
                args = Array<JSValue>::New(argCount);
                for (size_t index = argCount; index > 0; index--) {
                    args->Put(index - 1, self->Pop());
                }
                self->Pop();
            }
            Handle<JSValue> that = self->Pop();
            Handle<JSValue> callee = self->Pop();
            if (!Testing::IsCallable(callee)) {
                Exceptions::ThrowTypeError("Cannot call on a non-callable");
            }
            result = Objects::Call(callee.CastTo<JSObject>(), that, args);
            self->Push(result);
            break;
        }
        case Instruction::kNew: {
            Handle<Array<JSValue>> args;
            {
                Handle<JSValue> guard = GetPlaceholder();
                size_t size = self->stack->Size(), i;
                for (i = size - 1;; i--) {
                    if (self->stack->Get(i) == guard) {
                        break;
                    } else if (i == 0) {
                        assert(!"No array start placeholder found");
                    }
                }
                size_t argCount = size - i - 1;
                args = Array<JSValue>::New(argCount);
                for (size_t index = argCount; index > 0; index--) {
                    args->Put(index - 1, self->Pop());
                }
                self->Pop();
            }
            Handle<JSValue> callee = self->Pop();
            if (!Testing::IsConstructor(callee)) {
                Exceptions::ThrowTypeError("Cannot construct on a non-consturctor");
            }
            result = Objects::Construct(callee.CastTo<JSObject>(), args);
            self->Push(result);
            break;
        }

        case Instruction::kThrow: {
            throw ESException(self->Pop());
        }

        case Instruction::kArrayStart:
            result = GetPlaceholder();
            self->Push(result);
            break;
        case Instruction::kArrayElision:
            result = GetElisionPlaceholder();
            self->Push(result);
            break;
        case Instruction::kArray: {
            Handle<JSValue> guard = GetPlaceholder();
            Handle<JSValue> elision = GetElisionPlaceholder();

            size_t size = self->stack->Size(), i;
            for (i = size - 1;; i--) {
                if (self->stack->Get(i) == guard) {
                    break;
                } else if (i == 0) {
                    assert(!"No array start placeholder found");
                }
            }
            size_t arrayLen = size - i - 1;
            Handle<JSObject> array = Objects::ArrayCreate(arrayLen);
            for (size_t index = arrayLen; index > 0; index--) {
                Handle<JSValue> item = self->Pop();
                if (item != elision) {
                    Objects::CreateDataProperty(array, Conversion::ToString(JSNumber::New(static_cast<int64_t>(index - 1))), item);
                }
            }
            // Pop out the placeholder
            self->Pop();
            self->Push(array);
            break;
        }
        case Instruction::kSpread: {
            Handle<JSValue> spreadObj = self->Pop();
            Handle<JSObject> iterator = Iterators::GetIterator(spreadObj);
            while (Handle<JSObject> next = Iterators::IteratorStep(iterator)) {
                Handle<JSValue> nextValue = Iterators::IteratorValue(next);
                self->Push(nextValue);
            }
            break;
        }
        case Instruction::kDup:
            result = self->Peek();
            self->Push(result);
            break;
        case Instruction::kRotate3: {
            Handle<JSValue> third = self->Pop();
            Handle<JSValue> second = self->Pop();
            Handle<JSValue> first = self->Pop();
            self->Push(third);
            self->Push(first);
            self->Push(second);
            break;
        }
        case Instruction::kRotate4: {
            Handle<JSValue> fourth = self->Pop();
            Handle<JSValue> third = self->Pop();
            Handle<JSValue> second = self->Pop();
            Handle<JSValue> first = self->Pop();
            self->Push(fourth);
            self->Push(first);
            self->Push(second);
            self->Push(third);
            break;
        }
        case Instruction::kPop:
            self->Pop();
            break;
        case Instruction::kXchg: {
            Handle<JSValue> top1 = self->Pop();
            Handle<JSValue> top2 = self->Pop();
            self->Push(top1);
            self->Push(top2);
            break;
        }

        case Instruction::kImplicitThis: {
            uint16_t index = self->Fetch16();
            Handle<JSString> name = self->GetConstantAs<JSString>(index);

            Handle<Environment> lex = self->lexEnv;
            while (lex) {
                if (lex->HasBinding(name)) {
                    break;
                }
                lex = lex->outer();
            }

            if (lex) {
                result = lex->WithBaseObject();
            } else {
                throw "ReferenceError";
            }
            self->Push(result);
            break;
        }

        case Instruction::kThis: {
            result = self->ResolveThisBinding();
            self->Push(result);
            break;
        }

        case Instruction::kPrim: {
            Handle<JSValue> operand = self->Pop();
            operand = Conversion::ToPrimitive(operand);
            self->Push(operand);
            break;
        }
        case Instruction::kNum: {
            Handle<JSValue> operand = self->Pop();
            operand = Conversion::ToNumber(operand);
            self->Push(operand);
            break;
        }
        case Instruction::kStr: {
            Handle<JSValue> operand = self->Pop();
            operand = Conversion::ToString(operand);
            self->Push(operand);
            break;
        }
        case Instruction::kBool: {
            Handle<JSValue> operand = self->Pop();
            operand = Conversion::ToBoolean(operand);
            self->Push(operand);
            break;
        }
        case Instruction::kNeg: {
            result = JSNumber::New(-self->PopAs<JSNumber>()->Value());
            self->Push(result);
            break;
        }
        case Instruction::kBitwiseNot: {
            result = JSNumber::New(~Conversion::ToInt32(self->PopAs<JSNumber>()));
            self->Push(result);
            break;
        }
        case Instruction::kNot: {
            result = JSBoolean::New(!self->PopAs<JSBoolean>()->Value());
            self->Push(result);
            break;
        }
        case Instruction::kMul: {
            double rnum = self->PopAs<JSNumber>()->Value();
            double lnum = self->PopAs<JSNumber>()->Value();
            Handle<JSValue> result = JSNumber::New(lnum * rnum);
            self->Push(result);
            break;
        }
        case Instruction::kDiv: {
            double rnum = self->PopAs<JSNumber>()->Value();
            double lnum = self->PopAs<JSNumber>()->Value();
            Handle<JSValue> result = JSNumber::New(lnum / rnum);
            self->Push(result);
            break;
        }
        case Instruction::kMod: {
            double rnum = self->PopAs<JSNumber>()->Value();
            double lnum = self->PopAs<JSNumber>()->Value();
            Handle<JSValue> result = JSNumber::New(fmod(lnum, rnum));
            self->Push(result);
            break;
        }
        case Instruction::kAddGeneric: {
            Handle<JSValue> right = self->Pop();
            Handle<JSValue> left = self->Pop();
            left = Conversion::ToPrimitive(left);
            right = Conversion::ToPrimitive(right);

            if (left->GetType() == JSValue::Type::kString || right->GetType() == JSValue::Type::kString) {
                Handle<JSString> result = JSString::Concat(Conversion::ToString(left), Conversion::ToString(right));
                self->Push(result);
            } else {
                Handle<JSNumber> result =
                    JSNumber::New(
                        Conversion::ToNumber(left)->Value() +Conversion::ToNumber(right)->Value()
                    );
                self->Push(result);
            }
            break;
        }
        case Instruction::kSub: {
            double rnum = self->PopAs<JSNumber>()->Value();
            double lnum = self->PopAs<JSNumber>()->Value();
            Handle<JSValue> result = JSNumber::New(lnum - rnum);
            self->Push(result);
            break;
        }
        case Instruction::kShl: {
            uint32_t rnum = Conversion::ToUInt32(self->PopAs<JSNumber>());
            int32_t lnum = Conversion::ToUInt32(self->PopAs<JSNumber>());
            Handle<JSValue> result = JSNumber::New(lnum << (rnum&0x1F));
            self->Push(result);
            break;
        }
        case Instruction::kShr: {
            uint32_t rnum = Conversion::ToUInt32(self->PopAs<JSNumber>());
            int32_t lnum = Conversion::ToUInt32(self->PopAs<JSNumber>());
            Handle<JSValue> result = JSNumber::New(lnum >> (rnum & 0x1F));
            self->Push(result);
            break;
        }
        case Instruction::kUshr: {
            uint32_t rnum = Conversion::ToUInt32(self->PopAs<JSNumber>());
            uint32_t lnum = Conversion::ToUInt32(self->PopAs<JSNumber>());
            Handle<JSValue> result = JSNumber::New(static_cast<int64_t>(lnum >> (rnum & 0x1F)));
            self->Push(result);
            break;
        }
        case Instruction::kAnd: {
            int32_t rnum = Conversion::ToInt32(self->PopAs<JSNumber>());
            int32_t lnum = Conversion::ToInt32(self->PopAs<JSNumber>());
            Handle<JSValue> result = JSNumber::New(lnum & rnum);
            self->Push(result);
            break;
        }
        case Instruction::kXor: {
            int32_t rnum = Conversion::ToInt32(self->PopAs<JSNumber>());
            int32_t lnum = Conversion::ToInt32(self->PopAs<JSNumber>());
            Handle<JSValue> result = JSNumber::New(lnum ^ rnum);
            self->Push(result);
            break;
        }
        case Instruction::kOr: {
            int32_t rnum = Conversion::ToInt32(self->PopAs<JSNumber>());
            int32_t lnum = Conversion::ToInt32(self->PopAs<JSNumber>());
            Handle<JSValue> result = JSNumber::New(lnum | rnum);
            self->Push(result);
            break;
        }
        case Instruction::kLt: {
            Handle<JSValue> right = self->Pop();
            Handle<JSValue> left = self->Pop();
            assert(left->GetType() != JSValue::Type::kObject);
            assert(right->GetType() != JSValue::Type::kObject);
            bool ret = Testing::IsSmaller(left.CastTo<JSPrimitive>(), right.CastTo<JSPrimitive>()) == 1;
            self->Push(JSBoolean::New(ret));
            break;
        }
        case Instruction::kLteq: {
            Handle<JSValue> right = self->Pop();
            Handle<JSValue> left = self->Pop();
            assert(left->GetType() != JSValue::Type::kObject);
            assert(right->GetType() != JSValue::Type::kObject);
            bool ret = Testing::IsSmaller(right.CastTo<JSPrimitive>(), left.CastTo<JSPrimitive>()) == 0;
            self->Push(JSBoolean::New(ret));
            break;
        }
        case Instruction::kEq: {
            Handle<JSValue> right = self->Pop();
            Handle<JSValue> left = self->Pop();
            bool ret = Testing::IsAbstractlyEqual(left, right);
            self->Push(JSBoolean::New(ret));
            break;
        }
        case Instruction::kSeq: {
            Handle<JSValue> right = self->Pop();
            Handle<JSValue> left = self->Pop();
            bool ret = Testing::IsStrictlyEqual(left, right);
            self->Push(JSBoolean::New(ret));
            break;
        }

        case Instruction::kAdd: {
            Handle<JSValue> rightAsValue = self->Pop();
            Handle<JSValue> leftAsValue = self->Pop();
            Handle<JSNumber> left = Conversion::ToNumber(leftAsValue);
            Handle<JSNumber> right = Conversion::ToNumber(rightAsValue);

            Handle<JSNumber> result = JSNumber::New(left->Value() + right->Value());
            self->Push(result);
            break;
        }

        case Instruction::kConcat: {
            Handle<JSString> right = self->PopAs<JSString>();
            Handle<JSString> left = self->PopAs<JSString>();
            result = JSString::Concat(left, right);
            self->Push(result);
            break;
        }
        case Instruction::kDebugger: {
            break;
        }
        case Instruction::kReturn: {
            return ReturnStatus::kReturn;
        }
        case Instruction::kYield: {
            return ReturnStatus::kYield;
        }
        default:
            throw "unknown instruction";
    }
    return ReturnStatus::kNormal;
}

bool BytecodeContext::HandleException(const Handle<JSValue>& ex) {
    Handle<BytecodeContext> self = this;
    // -1 here because we advanced self->ip already in self->Step();
    uint16_t handler = self->code->FindExceptionHandler(self->ip - 1);
    if (handler != 0xFFFF) {
        // Clear the stack and push the exception
        self->stack->Clear();
        self->Push(ex);

        // Execute the push scope and pop scope in between
        int diff = self->code->FindScopeDifference(self->ip, handler);
        assert(diff>0);
        while (diff>0) {
            self->lexEnv = self->lexEnv->outer();
            diff--;
        }

        self->ip = handler;
        return true;
    } else {
        return false;
    }
}

BytecodeContext::ReturnStatus BytecodeContext::Run() {
    Handle<BytecodeContext> self = this;
    while (self->ip < self->code->Length()) {
        try {
            ReturnStatus status = self->Step();
            if (status != ReturnStatus::kNormal) {
                return status;
            }
        } catch (ESException& e) {
            if (!HandleException(e.value())) {
                throw;
            }
        }
    }
    throw "Unexpected out of instruction";
}