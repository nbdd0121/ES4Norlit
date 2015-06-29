#include "Instruction.h"
#include "Emitter.h"
#include "Code.h"


#include <cstdio>
#include <cstring>

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::bytecode;

Emitter::Emitter() {
    bytecode = ValueArray<uint8_t>::New(16);
}

size_t Emitter::EmitConstant(const Handle<JSValue>& val) {
    size_t size = constantPool.Size();
    for (size_t i = 0; i < size; i++) {
        if (constantPool.Get(i) == val) {
            return i;
        }
    }
    constantPool.Add(val);
    assert(size <= 0xFFFF);
    return size;
}


size_t Emitter::EmitCode(const Handle<Code>& val) {
    size_t ret = codePool.Size();
    codePool.Add(val);
    assert(ret <= 0xFFFF);
    return ret;
}

void Emitter::Emit8(uint8_t bc) {
    size_t capacity = bytecode->Length();
    if (bytecodeLength >= capacity) {
        Handle<ValueArray<uint8_t>> expanded = ValueArray<uint8_t>::New(capacity * 2);
        memcpy(&expanded->At(0), &bytecode->At(0), bytecodeLength);
        bytecode = expanded;
    }
    bytecode->At(bytecodeLength++) = bc;
}

void Emitter::Emit16(uint16_t data) {
    Emit8((data >> 8) & 0xFF);
    Emit8(data & 0xFF);
}

void Emitter::Emit(Instruction ins) {
    Emit8(static_cast<uint8_t>(ins));
}

Emitter::Placeholder Emitter::EmitPlaceholder() {
    size_t size = bytecodeLength;
    Emit16(0);
    return { static_cast<uint16_t>(size) };
}

Emitter::Label Emitter::EmitLabel() {
    return{ static_cast<uint16_t>(bytecodeLength) };
}

void Emitter::PatchLabel(Placeholder p, Label l) {
    bytecode->At(p.location) = (l.location >> 8)&0xFF;
    bytecode->At(p.location+1) = l.location & 0xFF;
}

void Emitter::NewExceptionTableEntry(Label start, Label end, Label handler) {
    exceptionTable.Add({
        start.location,
        end.location,
        handler.location
    });
}

Handle<Code> Emitter::ToCode() {
    Handle<Array<JSValue>> constant = constantPool.ToArray();
    Handle<Array<Code>> code = codePool.ToArray();
    Handle<ValueArray<Code::ExceptionTableEntry>> ex = exceptionTable.ToArray();
    Handle<ValueArray<uint8_t>> stripped = ValueArray<uint8_t>::New(bytecodeLength);
    memcpy(&stripped->At(0), &bytecode->At(0), bytecodeLength);
    return new Code(constant, code, ex, stripped);
}