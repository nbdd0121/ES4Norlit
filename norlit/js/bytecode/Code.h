#ifndef NORLIT_JS_BYTECODE_CODE_H
#define NORLIT_JS_BYTECODE_CODE_H

#include "../JSValue.h"
#include "../../gc/Array.h"

namespace norlit {
namespace js {
namespace bytecode {

class Code : public gc::Object {
  public:
    struct ExceptionTableEntry {
        uint16_t startPc;
        uint16_t endPc;
        uint16_t handlerPc;
    };
  private:
    gc::Array<JSValue>* constantPool = nullptr;
    gc::Array<Code>* codePool = nullptr;
    gc::ValueArray<ExceptionTableEntry>* exceptionTable = nullptr;
    gc::ValueArray<uint8_t>* bytecode = nullptr;

  public:

    Code(
        const gc::Handle<gc::Array<JSValue>>& constant,
        const gc::Handle<gc::Array<Code>>& code,
        const gc::Handle<gc::ValueArray<ExceptionTableEntry>>& exceptionTable,
        const gc::Handle<gc::ValueArray<uint8_t>>& bc
    ) {
        WriteBarrier(&constantPool, constant);
        WriteBarrier(&codePool, code);
        WriteBarrier(&this->exceptionTable, exceptionTable);
        WriteBarrier(&bytecode, bc);
    }

    uint8_t At(size_t ptr) {
        return bytecode->At(ptr);
    }
    size_t Length() {
        return bytecode->Length();
    }
    gc::Handle<JSValue> GetConstant(size_t ptr) {
        return constantPool->Get(ptr);
    }
    gc::Handle<Code> GetCode(size_t ptr) {
        return codePool->Get(ptr);
    }

    uint16_t FindExceptionHandler(uint16_t pc);
    int FindScopeDifference(uint16_t from, uint16_t to);

    void IterateField(const gc::FieldIterator&) override final;
    void Dump(size_t ident = 0);
};

}
}
}

#endif