#include "../all.h"

#include "Builtin.h"

#include "../vm/Context.h"
#include "../object/Exotics.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::builtin;

namespace {
Handle<JSObject> CreateArrayIterator(const Handle<JSObject>& array, ArrayIteratorObject::IterationKind kind) {
    Handle<ArrayIteratorObject> iterator = Objects::ObjectCreate<ArrayIteratorObject>(Context::CurrentRealm()->ArrayIteratorPrototype());
    iterator->iteratedObject(array);
    iterator->arrayIteratorNextIndex(0);
    iterator->arrayIterationKind(kind);
    return iterator;
}
}

Handle<JSValue> BuiltinArray::Call(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    return Construct(args, Context::CurrentContext()->function());
}

Handle<JSObject> BuiltinArray::Construct(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {
    size_t len = args->Length();
    Handle<JSObject> proto = Objects::GetPrototypeFromConstructor(target, &Realm::ArrayPrototype);
    if (len == 0) {
        return Objects::ArrayCreate(0, proto);
    } else if (len == 1 && Testing::Is<JSNumber>(GetArg(args, 0))) {
        Handle<JSNumber> len = GetArg(args, 0).CastTo<JSNumber>();
        uint32_t intLen = Conversion::ToUInt32(len);
        if (intLen != len->Value()) {
            Exceptions::ThrowRangeError("Array length out of range");
        }
        return Objects::ArrayCreate(intLen, proto);
    } else {
        Handle<JSObject> array = Objects::ArrayCreate(len, proto);
        for (size_t k = 0; k < len; k++) {
            Handle<JSString> Pk = Conversion::ToString(JSNumber::New(static_cast<int64_t>(k)));
            Handle<JSValue> itemK = args->Get(k);
            Objects::CreateDataProperty(array, Pk, itemK);
        }
        return array;
    }
}

Handle<JSValue> BuiltinArray::prototype::keys(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    return CreateArrayIterator(Conversion::ToObject(that), ArrayIteratorObject::IterationKind::kKey);
}

Handle<JSValue> BuiltinArray::prototype::values(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    return CreateArrayIterator(Conversion::ToObject(that), ArrayIteratorObject::IterationKind::kValue);
}

Handle<JSValue> BuiltinArray::Iterator_next(const Handle<JSValue>& that, const Handle<Array<JSValue>>& args) {
    if (!Testing::Is<JSObject>(that)) {
        Exceptions::ThrowIncompatibleReceiverTypeError("Array Iterator.prototype.next");
    }
    Handle<ArrayIteratorObject> O = that.ExactCheckedCastTo<ArrayIteratorObject>();
    if (!O) {
        Exceptions::ThrowIncompatibleReceiverTypeError("Array Iterator.prototype.next");
    }
    Handle<JSObject> a = O->iteratedObject();
    if (!a) {
        return Iterators::CreateIterResultObject(nullptr, true);
    }
    uint32_t index = O->arrayIteratorNextIndex();
    ArrayIteratorObject::IterationKind itemKind = O->arrayIterationKind();
    // 8. If a has a[[TypedArrayName]] internal slot, then
    //     a.Let len be the value of O¡¯s[[ArrayLength]] internal slot.
    // 9. Else,
    int64_t len = Conversion::ToLength(Objects::Get(a, "length"));
    if (index >= len) {
        O->iteratedObject(nullptr);
        return Iterators::CreateIterResultObject(nullptr, true);
    }
    O->arrayIteratorNextIndex(index + 1);
    Handle<JSNumber> keyAsNum = JSNumber::New(static_cast<int64_t>(index));
    if (itemKind == ArrayIteratorObject::IterationKind::kKey) {
        return Iterators::CreateIterResultObject(keyAsNum, false);
    }
    Handle<JSValue> elementValue = Objects::Get(a, Conversion::ToString(keyAsNum));
    if (itemKind == ArrayIteratorObject::IterationKind::kValue) {
        return Iterators::CreateIterResultObject(elementValue, false);
    } else {
        throw "return Iterators::CreateIterResultObject([index, elementValue], false);";
    }
}