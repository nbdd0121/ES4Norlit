#include "../all.h"

#include "JSOrdinaryObject.h"
#include "../../gc/Heap.h"

#include "../../util/Arrays.h"

#include "../JSSymbol.h"

#include <cassert>

using namespace norlit::gc;
using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::object;
using namespace norlit::util;

JSOrdinaryObject::JSOrdinaryObject(const Handle<JSObject>& prototype) {
    NoGC _;
    this->WriteBarrier(&this->propKey, new ArrayList<JSPropertyKey>());
    this->WriteBarrier(&this->propVal, new ArrayList<Property>());
    this->WriteBarrier(&this->prototype_, prototype);
}

int JSOrdinaryObject::LookupPropertyId(const Handle<JSPropertyKey>& P) {
    for (int i = 0, size = propKey->Size(); i < size; i++) {
        if (propKey->Get(i) == P) {
            return i;
        }
    }
    return -1;
}

Handle<Property> JSOrdinaryObject::LookupProperty(const Handle<JSPropertyKey>& P) {
    int index = LookupPropertyId(P);
    if (index == -1) {
        return{};
    }
    return propVal->Get((size_t)index);
}

/* 7.3.4 CreateDataProperty */
bool JSOrdinaryObject::CreateDataProperty(const Handle<JSObject>& O, const Handle<JSPropertyKey>& P, const Handle<JSValue>& V) {
    return O->DefineOwnProperty(P, {
        V,
        nullopt,
        nullopt,
        true,
        true,
        true
    });
}

Handle<JSObject> JSOrdinaryObject::GetPrototypeOf() {
    return{ prototype_ };
}

bool JSOrdinaryObject::SetPrototypeOf(const Handle<JSObject>& V) {
    Handle<JSOrdinaryObject> self = this;
    bool extensible = self->extensible;
    Handle<JSObject> current = self->prototype_;
    if (Testing::SameValue(V, current)) return true;
    if (!extensible) return false;
    Handle<JSObject> p = V;
    while (true) {
        if (!p) break;
        if (Testing::SameValue(p, self)) return false;
        Handle<JSOrdinaryObject> ordinaryP = p.DynamicCastTo<JSOrdinaryObject>();
        if (!ordinaryP) break;
        p = ordinaryP->prototype_;
    }
    self->prototype_ = V;
    return true;
}

bool JSOrdinaryObject::IsExtensible() {
    return extensible;
}

bool JSOrdinaryObject::PreventExtensions() {
    extensible = false;
    return true;
}

Optional<PropertyDescriptor> JSOrdinaryObject::GetOwnProperty(const Handle<JSPropertyKey>& p) {
    return OrdinaryGetOwnProperty(this, p);
}

Optional<PropertyDescriptor> JSOrdinaryObject::OrdinaryGetOwnProperty(const Handle<JSOrdinaryObject>& O, const Handle<JSPropertyKey>& P) {
    Handle<Property> X = O->LookupProperty(P);
    if (!X) {
        return{};
    }
    PropertyDescriptor D;
    if (X->IsDataProperty()) {
        D.value = X.CastTo<DataProperty>()->value;
        D.writable = X.CastTo<DataProperty>()->writable;
    } else {
        assert(X->IsAccessorProperty());
        D.get = X.CastTo<AccessorProperty>()->get;
        D.set = X.CastTo<AccessorProperty>()->set;
    }
    D.enumerable = X->enumerable;
    D.configurable = X->configurable;
    return D;
}

bool JSOrdinaryObject::DefineOwnProperty(const Handle<JSPropertyKey>& P, const PropertyDescriptor& Desc) {
    return OrdinaryDefineOwnProperty(this, P, Desc);
}

bool JSOrdinaryObject::OrdinaryDefineOwnProperty(const Handle<JSOrdinaryObject>& O, const Handle<JSPropertyKey>& P, const PropertyDescriptor& Desc) {
    return ValidateAndApplyPropertyDescriptor(O, P, O->extensible, Desc, O->GetOwnProperty(P));
}

bool JSOrdinaryObject::ValidateAndApplyPropertyDescriptor(
    const Handle<JSOrdinaryObject>& O,
    const Handle<JSPropertyKey>& P,
    bool extensible,
    const PropertyDescriptor& Desc,
    const Optional<PropertyDescriptor>& current
) {
    /* Tsukkomi: This is totally copy of spec */
    if (!current) {
        if (!extensible) {
            return false;
        }
        if (O) {
            Handle<Property> property;
            if (!Desc.IsAccessorDescriptor()) {
                Handle<DataProperty> prop = new DataProperty();
                prop->value = Desc.value ? (JSValue*)(*Desc.value) : nullptr;
                prop->writable = Desc.writable ? *Desc.writable : false;
                property = prop;
            } else {
                Handle<AccessorProperty> prop = new AccessorProperty();
                if (Desc.get) {
                    prop->get = *Desc.get;
                }
                if (Desc.set) {
                    prop->set = *Desc.set;
                }
                property = prop;
            }
            property->enumerable = Desc.enumerable ? *Desc.enumerable : false;
            property->configurable = Desc.configurable ? *Desc.configurable : false;
            O->propKey->Add(P);
            O->propVal->Add(property);
        }
        return true;
    }
    if (!Desc.value && !Desc.get && !Desc.set && !Desc.writable && !Desc.configurable && !Desc.enumerable) {
        return true;
    }
    do {
        if (Desc.value) if (!current->value || !Testing::SameValue(*Desc.value, *current->value)) break;
        if (Desc.get) if (!current->get || !Testing::SameValue(*Desc.get, *current->get)) break;
        if (Desc.set) if (!current->set || !Testing::SameValue(*Desc.set, *current->set)) break;
        if (Desc.writable) if (!current->writable || *Desc.writable != *current->writable) break;
        if (Desc.configurable) if (!current->configurable || *Desc.configurable != *current->configurable) break;
        if (Desc.enumerable) if (!current->enumerable || *Desc.enumerable != *current->enumerable) break;
        return true;
    } while (0);
    if (!*current->configurable) {
        if (Desc.configurable && *Desc.configurable) return false;
        if (Desc.enumerable && *Desc.enumerable != *current->enumerable) return false;
    }

    auto prop = O ? O->LookupProperty(P) : nullptr;

    if (Desc.IsGenericDescriptor()) {

    } else if (Desc.IsDataDescriptor() != current->IsDataDescriptor()) {
        if (!*current->configurable)return false;
        if (O) {
            Handle<Property> newProp;
            if (current->IsDataDescriptor()) {
                newProp = new AccessorProperty();
            } else {
                newProp = new DataProperty();
            }
            newProp->configurable = prop->configurable;
            newProp->enumerable = prop->enumerable;
            prop = newProp;
        }
    } else if (Desc.IsDataDescriptor()) {
        if (!*current->configurable) {
            if (!*current->writable) {
                if (*Desc.writable)return false;
                if (Desc.value&&!Testing::SameValue(*Desc.value, *current->value))return false;
            }
        }
    } else {
        if (!*current->configurable) {
            if (Desc.set&&!Testing::SameValue(*Desc.set, *current->set))return false;
            if (Desc.get&&!Testing::SameValue(*Desc.get, *current->get))return false;
        }
    }
    if (O) {
        if (prop->IsDataProperty()) {
            Handle<DataProperty> dProp = prop.CastTo<DataProperty>();
            if (Desc.value)dProp->value = *Desc.value;
            if (Desc.writable)dProp->writable = *Desc.writable;
        } else {
            Handle<AccessorProperty> aProp = prop.CastTo<AccessorProperty>();
            if (Desc.set)aProp->set = *Desc.set;
            if (Desc.get)aProp->get = *Desc.get;
        }
        if (Desc.configurable)prop->configurable = *Desc.configurable;
        if (Desc.enumerable)prop->enumerable = *Desc.enumerable;
    }
    return true;
}

bool JSOrdinaryObject::HasProperty(const Handle<JSPropertyKey>& P) {
    return OrdinaryHasProperty(this, P);
}

bool JSOrdinaryObject::OrdinaryHasProperty(const Handle<JSOrdinaryObject>& O, const Handle<JSPropertyKey>& P) {
    auto hasOwn = OrdinaryGetOwnProperty(O, P);
    if (hasOwn) {
        return true;
    }
    auto parent = O->GetPrototypeOf();
    if (parent) {
        return parent->HasProperty(P);
    }
    return false;
}

Handle<JSValue> JSOrdinaryObject::Get(const Handle<JSPropertyKey>& P, const Handle<JSValue>& Receiver) {
    Handle<JSOrdinaryObject> self = this;

    auto desc = self->GetOwnProperty(P);
    if (!desc) {
        auto parent = self->GetPrototypeOf();
        if (!parent) {
            return nullptr;
        }
        return parent->Get(P, Receiver);
    }
    if (desc->IsDataDescriptor()) {
        return *desc->value;
    } else {
        auto getter = desc->get;
        if (!getter) {
            return nullptr;
        } else {
            return Objects::Call(*getter, Receiver);
        }
    }
}

bool JSOrdinaryObject::Set(const Handle<JSPropertyKey>& P, const Handle<JSValue>& V, const Handle<JSValue>& Receiver) {
    Handle<JSOrdinaryObject> self = this;

    auto ownDesc = self->GetOwnProperty(P);
    if (!ownDesc) {
        auto parent = self->GetPrototypeOf();
        if (parent) {
            return parent->Set(P, V, Receiver);
        } else {
            ownDesc = PropertyDescriptor{
                { nullptr },
                nullopt,
                nullopt,
                true,
                true,
                true
            };
        }
    }
    if (ownDesc->IsDataDescriptor()) {
        if (ownDesc->writable && !*ownDesc->writable) return false;
        if (Receiver->GetType() != Type::kObject) return false;
        Handle<JSObject> ReceiverO = Receiver.CastTo<JSObject>();
        auto existingDescriptor = ReceiverO->GetOwnProperty(P);
        if (existingDescriptor) {
            PropertyDescriptor valueDesc{
                V,
                nullopt,
                nullopt,
                nullopt,
                nullopt,
                nullopt
            };
            return ReceiverO->DefineOwnProperty(P, valueDesc);
        } else {
            return CreateDataProperty(ReceiverO, P, V);
        }
    }
    assert(ownDesc->IsAccessorDescriptor());
    if (!ownDesc->set) {
        return false;
    }
    Objects::Call(*ownDesc->set, Receiver, Arrays::ToArray<JSValue>(V));
    return true;
}

bool JSOrdinaryObject::Delete(const Handle<JSPropertyKey>& P) {
    Handle<JSOrdinaryObject> self = this;

    auto desc = self->GetOwnProperty(P);
    if (!desc) return true;
    if (*desc->configurable) {
        int index = self->LookupPropertyId(P);
        self->propKey->Remove(index);
        self->propVal->Remove(index);
        return true;
    } else {
        return false;
    }
}

// TODO Get rid of this stupid std::vector
#include <vector>
#include <algorithm>

Handle<Array<JSPropertyKey>> JSOrdinaryObject::OwnPropertyKeys() {
    Handle<ArrayList<JSPropertyKey>> keys = this->propKey;

    ArrayList<JSPropertyKey> list;
    std::vector<int64_t> indexes;

    size_t size = keys->Size();
    for (size_t i = 0; i < size; i++) {
        if (Handle<JSString> key = Testing::CastIf<JSString>(keys->Get(i))) {
            int64_t index = Conversion::ToIntegerIndex(key);
            if (index != -1) indexes.push_back(index);
        }
    }

    std::sort(indexes.begin(), indexes.end());

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

void JSOrdinaryObject::IterateField(const FieldIterator& callback) {
    callback(&propKey);
    callback(&propVal);
    callback(&prototype_);
}