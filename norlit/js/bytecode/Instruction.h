#ifndef NORLIT_JS_BYTECODE_INSTRUCTION_H
#define NORLIT_JS_BYTECODE_INSTRUCTION_H

#include <cstdint>

namespace norlit {
namespace js {
namespace bytecode {

enum class Instruction: uint8_t {
    // Immediates            uint16_t name
    // Create a mutable binding in lexical environment with given name and initialize to undefined
    kDefVar,

    // Immediates            uint16_t name
    // Create a mutable binding in lexical environment with given name. It will be left uninitialized
    kDefLet,

    // Immediates            uint16_t name
    // Create a immutable binding in lexical environment with given name. It will be left uninitialized
    kDefConst,

    // Precondition     ... [Operand: Any]
    // Postcondition    ...
    // Immediates            uint16_t name
    // Initialize the lexical binding with operand poped out from stack
    kInitDef,

    // Precondition     ...
    // Postcondition    ... [Result: Any]
    // Immediates            uint16_t name
    // Load the constant from constant pool
    kLoad,

    // Precondition     ...
    // Postcondition    ... [Result: Any]
    // Immediates            uint16_t name
    // If the identifier cannot be resolved, throw a ReferenceError
    // Otherwise push the binding value onto the stack
    kGetName,

    // Precondition     ...
    // Postcondition    ... [Result: Any]
    // Immediates            uint16_t name
    // Evaluate typeof(name) === 'undefined' ? undefined : name
    kGetNameOrUndef,

    // Precondition     ... [Operand1: Any]
    // Postcondition    ... [Operand1: Any]
    // Immediates            uint16_t name
    // Evaluate name = Operand1
    kPutName,

    // Precondition     ...
    // Postcondition    ... [Result: Boolean]
    // Immediates            uint16_t name
    // Perform eval("delete "+name)
    kDeleteName,

    // Precondition     ...
    // Postcondition    ... [Result: Any]
    // Immediates           uint16_t name
    // If the identifier cannot be resolved, throw a ReferenceError
    // Otherwise push the implicit this value of the identifier
    kImplicitThis,

    // Immediates            uint16_t target
    // Jump to instruction at target position
    kJump,

    // Precondition     ... [Operand1: Boolean]
    // Postcondition    ...
    // Immediates            uint16_t target
    // Pop a boolean, jump to instruction at target position if operand is true
    kJumpIfTrue,

    kFunction,
    kGenerator,

    kUndef,
    kTrue,
    kOne,

    kXchg,
    kPrim,
    kNum,
    kStr,
    kBool,

    // Precondition     ... [Operand1: Any]
    // Postcondition    ... [Result: String]
    // Pop the operand and push typeof(Operand1)
    kTypeOf,

    // Precondition     ... [Operand1: Any] [Operand2: Any]
    // Postcondtion     ... [Result: Any]
    // Pop two operands, evaluate Operand1[Operand2] and push result to stack
    kGetProperty,

    // Precondition     ... [Operand1: Any] [Operand2: Any]
    // Postcondtion     ... [Operand1: String, Number, Boolean, Symbol or Object] [Operand2 -> String or Symbol] [Result: Any]
    // Convert Operand2 to property key, evaluate Operand1[Operand2] and push result to stack
    kGetPropertyNoPop,

    // Precondition     ... [Operand1: Any] [Operand2: Any] [Operand3: Any]
    // Postcondition    ... [Operand3: Any]
    // Pop three operands, evaluate Operand1[Operand2] = Operand3 and push operand 3 back to stack
    kSetProperty,

    // Precondition     ... [Operand1: Any] [Operand2: Any]
    // Postcondition    ... [Result: Boolean]
    // Evaluate and push { delete Operand1[Operand2] }
    kDeleteProperty,

    // Precondition     ... [Operand1: Object] [Operand2: Any] [Operand3: Any]
    // Postcondition    ... [Operand1: Object]
    // Pop three operands, evaluate Object.defineProperty(Operand1, Operand2, {value: Operand3, writable: true, enumerable: true, configurable: true})
    // and push Operand1 back to stack
    kCreateDataProperty,

    // Precondition     ...
    // Postcondition    ... [Result: Object]
    // Push new Object()
    kCreateObject,

    kArrayStart,
    kArrayElision,
    kArray,
    kSpread,
    kCall,
    kNew,

    kPushScope,
    kPopScope,

    kThrow,
    kInstanceOf,

    kThis,

    kNeg,
    kBitwiseNot,
    kNot,
    kMul,
    kDiv,
    kMod,

    // Precondition	    ... [Operand1: Any -> Primitive] [Operand2: Any -> Primitive]
    // Postcondition    ... [Result: Primitive]
    // Pop two primitives from stack, add them and push to stack
    // If any of the two operand is string, it will be a string concatenation
    kAddGeneric,

    kSub,
    kShl,
    kShr,
    kUshr,
    kLt,
    kLteq,
    kEq,
    kSeq,
    kAnd,
    kXor,
    kOr,

    // Precondition	    ... [Operand1: Any -> Number] [Operand2: Any -> Number]
    // Postcondition    ... [Result: Number]
    // Pop two numbers from stack, add them and push to stack
    kAdd,

    // Precondition	    ... [Operand1: Any -> String] [Operand2: Any -> String]
    // Postcondition    ... [Result: String]
    // Pop two strings from stack, concat them and push to stack
    kConcat,

    // Precondition     ... [Operand1: Any]
    // Postcondition    ... [Operand1] [Operand1]
    // Duplicate the stack top
    kDup,

    // Precondition     ... [Operand1: Any]
    // Postcondition    ...
    // Pop the stack top
    kPop,

    kRotate3,
    kRotate4,

    kReturn,
    kYield,

    // Trigger debugger
    kDebugger
};

}
}
}

#endif