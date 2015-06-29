#include "../all.h"

#include "Exotics.h"

using namespace norlit::gc;
using namespace norlit::util;
using namespace norlit::js;
using namespace norlit::js::object;

void SymbolObject::IterateField(const FieldIterator& iter) {
    JSOrdinaryObject::IterateField(iter);
    iter(&this->symbolData_);
}

Optional<PropertyDescriptor> StringObject::GetOwnProperty(const Handle<JSPropertyKey>& key) {
    Handle<StringObject> self = this;
    Optional<PropertyDescriptor> desc = OrdinaryGetOwnProperty(self, key);
    if (desc) {
        return desc;
    }
    if (Handle<JSString> keyAsStr = Testing::CastIf<JSString>(key)) {
        int64_t index = Conversion::ToIntegerIndex(keyAsStr);
        if (index != -1) {
            size_t len = self->stringData_->Length();
            if (index < static_cast<int64_t>(len)) {
                return PropertyDescriptor{
                    { self->stringData_->Substring(static_cast<size_t>(index), static_cast<size_t>(index + 1)) },
                    nullopt,
                    nullopt,
                    true,
                    false,
                    false
                };
            }
        }
    }
    return nullopt;
}

bool StringObject::HasProperty(const Handle<JSPropertyKey>& key) {
    Handle<StringObject> self = this;
    if (Handle<JSString> keyAsStr = Testing::CastIf<JSString>(key)) {
        int64_t index = Conversion::ToIntegerIndex(keyAsStr);
        if (index != -1) {
            size_t len = self->stringData_->Length();
            if (index < static_cast<int64_t>(len)) {
                return true;
            }
        }
    }
    return OrdinaryHasProperty(self, key);
}

// TODO Get rid of this stupid std::vector
#include <vector>
#include <algorithm>

Handle<Array<JSPropertyKey>> StringObject::OwnPropertyKeys() {
    Handle<ArrayList<JSPropertyKey>> keys = this->propKey;
    size_t len = this->stringData_->Length();

    ArrayList<JSPropertyKey> list;
    std::vector<int64_t> indexes;

    size_t size = keys->Size();
    for (size_t i = 0; i < size; i++) {
        if (Handle<JSString> key = Testing::CastIf<JSString>(keys->Get(i))) {
            int64_t index = Conversion::ToIntegerIndex(key);
            if (index >= static_cast<int64_t>(len))indexes.push_back(len);
        }
    }

    std::sort(indexes.begin(), indexes.end());

    for (size_t i = 0; i < len; i++) {
        list.Add(Conversion::ToString(JSNumber::New(static_cast<int64_t>(i))));
    }

    for (int64_t i : indexes) {
        list.Add(Conversion::ToString(JSNumber::New(i)));
    }

    for (size_t i = 0; i < size; i++) {
        if (Handle<JSString> key = Testing::CastIf<JSString>(keys->Get(i))) {
            int64_t index = Conversion::ToIntegerIndex(key);
            if (index == -1) list.Add(key);
        }
    }

    for (size_t i = 0; i < size; i++) {
        if (Handle<JSSymbol> key = Testing::CastIf<JSSymbol>(keys->Get(i))) {
            list.Add(key);
        }
    }

    return list.ToArray();
}

void StringObject::IterateField(const FieldIterator& iter) {
    JSOrdinaryObject::IterateField(iter);
    iter(&this->stringData_);
}

bool ArrayObject::DefineOwnProperty(const Handle<JSPropertyKey>& P, const PropertyDescriptor& Desc) {
    Handle<ArrayObject> self = this;
    if (P == JSString::New("length")) {
        if (!Desc.value) {
            return OrdinaryDefineOwnProperty(self, P, Desc);
        }
        PropertyDescriptor newLenDesc = Desc;
        int64_t newLen = Conversion::ToUInt32(Conversion::ToNumber(*Desc.value));
        double numberLen = Conversion::ToNumberValue(*Desc.value);
        if (newLen != numberLen) {
            throw "Throw range error";
        }
        newLenDesc.value = JSNumber::New(newLen);
        Optional<PropertyDescriptor> oldLenDesc = OrdinaryGetOwnProperty(self, P);
        int64_t oldLen = Conversion::ToUInt32(Conversion::ToNumber(*Desc.value));
        if (newLen >= oldLen) {
            return OrdinaryDefineOwnProperty(self, P, newLenDesc);
        }
        if (!*oldLenDesc->writable) {
            return false;
        }
        bool newWritable;
        if (!newLenDesc.writable || *newLenDesc.writable) {
            newWritable = true;
        } else {
            newWritable = false;
            newLenDesc.writable = true;
        }

        bool succeeded = OrdinaryDefineOwnProperty(self, P, newLenDesc);
        if (!succeeded) return false;
        while (newLen < oldLen) {
            oldLen--;
            Handle<JSString> key = Conversion::ToString(JSNumber::New(oldLen));
            bool deleteSucceeded = self->Delete(key);
            if (deleteSucceeded == false) {
                newLenDesc.value = JSNumber::New(oldLen + 1);
                if (!newWritable) {
                    newLenDesc.writable = false;
                }
                OrdinaryDefineOwnProperty(self, P, newLenDesc);
                return false;
            }
        }
        if (newWritable == false) {
            OrdinaryDefineOwnProperty(self, P, {
                nullopt,
                nullopt,
                nullopt,
                false,
                nullopt,
                nullopt,
            });
        }
        return true;
    } else {
        int64_t PAsIndex = Conversion::ToIntegerIndex(P);
        if (PAsIndex != -1 && PAsIndex < 0xFFFFFFFF) {
            Optional<PropertyDescriptor> oldLenDesc = OrdinaryGetOwnProperty(self, JSString::New("length"));
            uint32_t oldLen = Conversion::ToUInt32(Conversion::ToNumber(*oldLenDesc->value));
            if (PAsIndex >= oldLen && *oldLenDesc->writable == false) {
                return false;
            }
            bool succeeded = OrdinaryDefineOwnProperty(self, P, Desc);
            if (!succeeded)return succeeded;
            if (PAsIndex >= oldLen) {
                oldLenDesc->value = JSNumber::New(PAsIndex + 1);
                OrdinaryDefineOwnProperty(self, JSString::New("length"), *oldLenDesc);
                return true;
            }
        }
    }
    return OrdinaryDefineOwnProperty(self, P, Desc);

}

void ArrayIteratorObject::IterateField(const FieldIterator& iter) {
    JSOrdinaryObject::IterateField(iter);
    iter(&this->iteratedObject_);
}

void GeneratorObject::IterateField(const FieldIterator& iter) {
    JSOrdinaryObject::IterateField(iter);
    iter(&this->generatorContext_);
}