#ifndef NORLIT_JS_GRAMMAR_TOKEN_H
#define NORLIT_JS_GRAMMAR_TOKEN_H

#include "../JSValue.h"
#include "../../gc/Handle.h"

namespace norlit {
namespace js {
namespace grammar {

class Token: public gc::Object {
  public:
    enum {
        kEllipse = 0x200,
        kStrictEq,
        kStrictIneq,
        kURShift,
        kLShiftAssign,
        kRShiftAssign,
        kURShiftAssign,
        kLambda,
        kEOF,
        kString,
        kNumber,
        kIdentifier,
        kNoSubTemplate,
        kTemplateHead,
        kTemplateMiddle,
        kTemplateTail,
        kRegexp,

        kBreak,
        kCase,
        kCatch,
        kClass,
        kConst,
        kContinue,
        kDebugger,
        kDefault,
        kDelete,
        kDo,
        kElse,
        kExport,
        kExtends,
        kFinally,
        kFor,
        kFunction,
        kIf,
        kImport,
        kIn,
        kInstanceof,
        kNew,
        kReturn,
        kSuper,
        kSwitch,
        kThis,
        kThrow,
        kTry,
        kTypeof,
        kVar,
        kVoid,
        kWhile,
        kWith,
        kYield,
        kNull,
        kTrue,
        kFalse,
        kReserved,

        kLteq = '<' + 0x80,
        kGteq = '>' + 0x80,
        kEq = '=' + 0x80,
        kIneq = '!' + 0x80,
        kAddAssign = '+' + 0x80,
        kSubAssign = '-' + 0x80,
        kMulAssign = '*' + 0x80,
        kDivAssign = '/' + 0x80,
        kModAssign = '%' + 0x80,
        kAndAssign = '&' + 0x80,
        kOrAssign = '|' + 0x80,
        kXorAssign = '^' + 0x80,

        kInc = '+' + 0x100,
        kDec = '-' + 0x100,
        kLShift = '<' + 0x100,
        kRShift = '>' + 0x100,
        kLAnd = '&' + 0x100,
        kLOr = '|' + 0x100,
    };
    enum {
        kEscaped = 1,
        kLegacy = 2,
        kLineBefore = 4,
    };

  public:
    JSValue* value = nullptr;
    JSValue* additional = nullptr;
    uint16_t type;
    uint8_t flags;

  public:
    Token(uint16_t type, uint8_t flags = 0, const gc::Handle<JSValue>& value = nullptr, const gc::Handle<JSValue>& additional = nullptr) :type(type), flags(flags) {
        WriteBarrier(&this->value, value);
        WriteBarrier(&this->additional, additional);
    }

    virtual void IterateField(const gc::FieldIterator& iter) override final {
        iter(&value);
        iter(&additional);
    }
};

}
}
}

#endif
