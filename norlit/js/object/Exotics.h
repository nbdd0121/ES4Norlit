#ifndef NORLIT_JS_OBJECT_EXOTICS_H
#define NORLIT_JS_OBJECT_EXOTICS_H

#include "../common.h"
#include "JSOrdinaryObject.h"

namespace norlit {
namespace js {
namespace object {

class BooleanObject final : public JSOrdinaryObject {
    bool booleanData_;
  public:
    BooleanObject(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}

    bool booleanData() const {
        return booleanData_;
    }

    void booleanData(bool data) {
        booleanData_ = data;
    }
};

class SymbolObject final : public JSOrdinaryObject {
    JSSymbol* symbolData_ = nullptr;
  public:
    SymbolObject(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}

    gc::Handle<JSSymbol> symbolData() const {
        return symbolData_;
    }

    void symbolData(const gc::Handle<JSSymbol>& data) {
        this->WriteBarrier(&this->symbolData_, data);
    }

    virtual void IterateField(const gc::FieldIterator& iter);
};

class ErrorObject final : public JSOrdinaryObject {
  public:
    ErrorObject(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}
};

class NumberObject final : public JSOrdinaryObject {
    double numberData_;
  public:
    NumberObject(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}

    double numberData() const {
        return numberData_;
    }

    void numberData(double data) {
        numberData_ = data;
    }
};

class DateObject final : public JSOrdinaryObject {
    int64_t dateValue_;
  public:
    DateObject(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}

    int64_t dateValue() const {
        return dateValue_;
    }

    void dateValue(int64_t data) {
        dateValue_ = data;
    }
};

class StringObject final : public JSOrdinaryObject {
    JSString* stringData_;
  public:
    StringObject(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}

    gc::Handle<JSString> stringData() const {
        return stringData_;
    }

    void stringData(const gc::Handle<JSString>& data) {
        this->WriteBarrier(&this->stringData_, data);
    }

    virtual util::Optional<PropertyDescriptor> GetOwnProperty(const gc::Handle<JSPropertyKey>&) override;
    virtual bool HasProperty(const gc::Handle<JSPropertyKey>&) override;
    virtual gc::Handle<gc::Array<JSPropertyKey>> OwnPropertyKeys() override;
    virtual void IterateField(const gc::FieldIterator&) override;
};

class RegExpObject final : public JSOrdinaryObject {
  public:
    RegExpObject(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}

    // TODO
};

class ArrayObject final : public JSOrdinaryObject {
  public:
    ArrayObject(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}

    virtual bool DefineOwnProperty(const gc::Handle<JSPropertyKey>&, const PropertyDescriptor&) override;
};

class ArrayIteratorObject final : public JSOrdinaryObject {
  public:
    enum class IterationKind {
        kKey,
        kValue,
        kBoth
    };
  private:
    JSObject* iteratedObject_;
    uint32_t arrayIteratorNextIndex_;
    IterationKind arrayIterationKind_;
  public:
    ArrayIteratorObject(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}

    gc::Handle<JSObject> iteratedObject() const {
        return iteratedObject_;
    }

    void iteratedObject(const gc::Handle<JSObject>& data) {
        this->WriteBarrier(&this->iteratedObject_, data);
    }

    uint32_t arrayIteratorNextIndex() const {
        return arrayIteratorNextIndex_;
    }

    void arrayIteratorNextIndex(uint32_t index) {
        arrayIteratorNextIndex_ = index;
    }

    IterationKind arrayIterationKind() const {
        return arrayIterationKind_;
    }

    void arrayIterationKind(IterationKind kind) {
        arrayIterationKind_ = kind;
    }

    virtual void IterateField(const gc::FieldIterator&) override;
};

class GeneratorObject final: public JSOrdinaryObject {
  public:
    enum class GeneratorState {
        kSuspendedStart,
        kSuspendedYield,
        kExecuting,
        kCompleted
    };
    NORLIT_DEFINE_FIELD(vm::BytecodeContext, generatorContext);
    NORLIT_DEFINE_FIELD_POD(GeneratorState, generatorState);
  public:
    GeneratorObject(const gc::Handle<JSObject>& proto) :JSOrdinaryObject(proto) {}

    virtual void IterateField(const gc::FieldIterator&) override;
};

}
}
}

#endif