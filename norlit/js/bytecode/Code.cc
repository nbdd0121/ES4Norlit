#include "Instruction.h"
#include "Code.h"

#include "../JSString.h"
#include "../JSNumber.h"
#include "../JSBoolean.h"

#include <cstdio>

#define IDENT(s) printf("\n%*s", static_cast<int>(ident + s), "")

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::bytecode;

namespace {
bool HasImmediate(Instruction ins) {
    switch (ins) {
        case Instruction::kDefVar:
        case Instruction::kDefLet:
        case Instruction::kDefConst:
        case Instruction::kInitDef:
        case Instruction::kLoad:
        case Instruction::kGetName:
        case Instruction::kGetNameOrUndef:
        case Instruction::kPutName:
        case Instruction::kDeleteName:
        case Instruction::kImplicitThis:
        case Instruction::kJump:
        case Instruction::kJumpIfTrue:
        case Instruction::kFunction:
        case Instruction::kGenerator:
            return true;
        default:
            return false;
    }
}
}

void Code::IterateField(const FieldIterator& iter) {
    iter(&this->constantPool);
    iter(&this->codePool);
    iter(&this->exceptionTable);
    iter(&this->bytecode);
}

uint16_t Code::FindExceptionHandler(uint16_t pc) {
    for (size_t len = this->exceptionTable->Length(), i = 0; i < len; i++) {
        const ExceptionTableEntry& ent = this->exceptionTable->At(i);
        if (ent.startPc <= pc&&pc < ent.endPc) {
            return ent.handlerPc;
        }
    }
    return 0xFFFF;
}

int Code::FindScopeDifference(uint16_t from, uint16_t to) {
    if (from > to)
        return -this->FindScopeDifference(to, from);
    int ret = 0;
    for (size_t i = from; i < to; i++) {
        Instruction ins = static_cast<Instruction>(this->bytecode->At(i));
        switch (ins) {
            case Instruction::kPushScope:
                ret--;
                break;
            case Instruction::kPopScope:
                ret++;
                break;
            default:
                if (HasImmediate(ins)) {
                    i += 2;
                }
                break;
        }
    }
    return ret;
}

void Code::Dump(size_t ident) {
    Handle<Array<JSValue>> constant = this->constantPool;
    Handle<Array<Code>> code = this->codePool;
    Handle<ValueArray<ExceptionTableEntry>> exceptionTable = this->exceptionTable;
    Handle<ValueArray<uint8_t>> bc = this->bytecode;
    printf("Code");
    IDENT(2);
    printf("Constant Pool");
    for (size_t i = 0, size = constant->Length(); i < size; i++) {
        IDENT(4);
        Handle<JSValue> literal = constant->Get(i);
        switch (literal->GetType()) {
            case JSValue::Type::kString: {
                Handle<JSString> str = literal.CastTo<JSString>();
                printf("%s", &str->ToCString()->At(0));
                break;
            }
            case JSValue::Type::kNumber: {
                Handle<JSNumber> val = literal.CastTo<JSNumber>();
                printf("%lf", val->Value());
                break;
            }
            case JSValue::Type::kBoolean: {
                Handle<JSBoolean> val = literal.CastTo<JSBoolean>();
                printf("%s", val->Value() ? "true" : "false");
                break;
            }
            case JSValue::Type::kNull: {
                printf("null");
                break;
            }
            default:
                throw "UNK";
        }
    }

    IDENT(2);
    printf("Code Pool");
    for (size_t i = 0, size = code->Length(); i < size; i++) {
        IDENT(4);
        code->Get(i)->Dump(ident + 4);
    }

    IDENT(2);
    printf("Bytecode");
    for (size_t i = 0, size = bc->Length(); i < size; i++) {
        auto get16 = [&] () {
            uint8_t hi = bc->At(++i);
            uint8_t lo = bc->At(++i);
            return (hi << 8) | lo;
        };

        IDENT(4);
        printf("%-3d ", static_cast<int>(i));

        Instruction ins = static_cast<Instruction>(bc->At(i));
        switch (ins) {
            case Instruction::kDefVar:
                printf("var %d", get16());
                break;
            case Instruction::kDefConst:
                printf("const %d", get16());
                break;
            case Instruction::kInitDef:
                printf("init %d", get16());
                break;
            case Instruction::kLoad:
                printf("load %d", get16());
                break;
            case Instruction::kGetName:
                printf("get_name %d",get16());
                break;
            case Instruction::kGetNameOrUndef:
                printf("get_name_or_undef %d", get16());
                break;
            case Instruction::kPutName:
                printf("put_name %d", get16());
                break;
            case Instruction::kImplicitThis:
                printf("implicit_this %d", get16());
                break;
            case Instruction::kJump:
                printf("jump %d", get16());
                break;
            case Instruction::kJumpIfTrue:
                printf("jump_if_true %d",get16());
                break;
            case Instruction::kFunction:
                printf("function %d", get16());
                break;
            case Instruction::kOne:
                printf("one");
                break;
            case Instruction::kThis:
                printf("this");
                break;
            case Instruction::kArrayStart:
                printf("array_start");
                break;
            case Instruction::kArrayElision:
                printf("array_elision");
                break;
            case Instruction::kArray:
                printf("array");
                break;
            case Instruction::kSpread:
                printf("spread");
                break;
            case Instruction::kCall:
                printf("call");
                break;
            case Instruction::kNew:
                printf("new");
                break;
            case Instruction::kSetProperty:
                printf("set_property");
                break;
            case Instruction::kGetProperty:
                printf("get_property");
                break;
            case Instruction::kGetPropertyNoPop:
                printf("get_property_no_pop");
                break;
            case Instruction::kTypeOf:
                printf("typeof");
                break;
            case Instruction::kUndef:
                printf("undef");
                break;
            case Instruction::kXchg:
                printf("xchg");
                break;
            case Instruction::kPrim:
                printf("prim");
                break;
            case Instruction::kNum:
                printf("num");
                break;
            case Instruction::kStr:
                printf("str");
                break;
            case Instruction::kBool:
                printf("bool");
                break;
            case Instruction::kNeg:
                printf("neg");
                break;
            case Instruction::kBitwiseNot:
                printf("bitwise_not");
                break;
            case Instruction::kNot:
                printf("not");
                break;
            case Instruction::kMul:
                printf("mul");
                break;
            case Instruction::kDiv:
                printf("div");
                break;
            case Instruction::kMod:
                printf("mod");
                break;
            case Instruction::kAddGeneric:
                printf("add");
                break;
            case Instruction::kSub:
                printf("sub");
                break;
            case Instruction::kShl:
                printf("shl");
                break;
            case Instruction::kShr:
                printf("shr");
                break;
            case Instruction::kUshr:
                printf("ushr");
                break;
            case Instruction::kLt:
                printf("lt");
                break;
            case Instruction::kLteq:
                printf("lteq");
                break;
            case Instruction::kEq:
                printf("eq");
                break;
            case Instruction::kSeq:
                printf("seq");
                break;
            case Instruction::kAnd:
                printf("and");
                break;
            case Instruction::kXor:
                printf("xor");
                break;
            case Instruction::kOr:
                printf("or");
                break;
            case Instruction::kPop:
                printf("pop");
                break;
            case Instruction::kDup:
                printf("dup");
                break;
            case Instruction::kConcat:
                printf("concat");
                break;
            case Instruction::kDebugger:
                printf("debugger");
                break;
            case Instruction::kReturn:
                printf("return");
                break;
            case Instruction::kThrow:
                printf("throw");
                break;
            case Instruction::kPushScope:
                printf("push_scope");
                break;
            case Instruction::kPopScope:
                printf("pop_scope");
                break;
            default:
                printf("Unknown Instruction");
        }
    }

    IDENT(2);
    printf("Exception Handling Table");
    IDENT(4);
    printf("Start End   Handler");
    for (size_t len = exceptionTable->Length(), i = 0; i < len; i++) {
        IDENT(4);
        const ExceptionTableEntry& ent = exceptionTable->At(i);
        printf("%05d %05d %05d", ent.startPc, ent.endPc, ent.handlerPc);
    }
}