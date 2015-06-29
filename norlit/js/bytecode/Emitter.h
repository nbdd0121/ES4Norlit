#ifndef NORLIT_JS_BYTECODE_EMITTER_H
#define NORLIT_JS_BYTECODE_EMITTER_H

#include "Code.h"

#include "../JSValue.h"
#include "../../gc/Array.h"
#include "../../util/ArrayList.h"
#include "../../util/ValueArrayList.h"

namespace norlit {
namespace js {
namespace bytecode {

enum class Instruction: uint8_t;

    class Emitter {
    util::ArrayList<JSValue> constantPool;
    util::ArrayList<Code> codePool;
    util::ValueArrayList<Code::ExceptionTableEntry> exceptionTable;
    gc::Handle<gc::ValueArray<uint8_t>> bytecode;
    size_t bytecodeLength = 0;

  public:
    struct Label {
        uint16_t location;
    };
    struct Placeholder {
        uint16_t location;
    };

    Emitter();
    size_t EmitConstant(const gc::Handle<JSValue>& val);
    size_t EmitCode(const gc::Handle<Code>& val);
    void Emit8(uint8_t byte);
    void Emit16(uint16_t data);
    void Emit(Instruction ins);
    Placeholder EmitPlaceholder();
    Label EmitLabel();
    void PatchLabel(Placeholder, Label);

    void NewExceptionTableEntry(Label, Label, Label);

    gc::Handle<Code> ToCode();
};

}
}
}

#endif