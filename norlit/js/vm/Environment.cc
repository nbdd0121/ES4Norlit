#include "../all.h"

#include "Environment.h"

#include "../../gc/Heap.h"
#include "../JSString.h"
#include "../JSSymbol.h"
#include "../object/JSObject.h"
#include "../object/JSFunction.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::util;

Environment::Environment(const Handle<Environment>& outer) {
    this->WriteBarrier(&this->outer_, outer);
}

Handle<JSValue> Environment::GetThisBinding() {
    throw "Assertion failure";
}

void Environment::IterateField(const FieldIterator& iter) {
    iter(&this->outer_);
}

struct DeclarativeEnvironemnt::Entry: public Object {
  private:
    JSValue* value_ = nullptr;
    bool strict_;
  public:
    bool mutable_;
    bool initialized = false;

    Handle<JSValue> value() {
        return value_;
    }

    void value(const gc::Handle<JSValue>& value) {
        this->WriteBarrier(&this->value_, value);
    }

    bool strict() {
        if (mutable_)
            return false;
        else
            return strict_;
    }

    bool canDelete() {
        if (mutable_)
            return strict_;
        else
            return false;
    }

    Entry(bool mutable_, bool strict) :strict_(strict), mutable_(mutable_) {

    }

    virtual void IterateField(const FieldIterator& iter) {
        iter(&this->value_);
    }
};

DeclarativeEnvironemnt::DeclarativeEnvironemnt(const Handle<Environment>& outer):Environment(outer) {
    NoGC _;
    this->WriteBarrier(&this->entries, new HashMap<JSString, Entry>());
}

bool DeclarativeEnvironemnt::HasBinding(const Handle<JSString>& key) {
    if (this->entries->Get(key)) {
        return true;
    } else {
        return false;
    }
}

void DeclarativeEnvironemnt::CreateMutableBinding(const Handle<JSString>& key, bool canDelete) {
    Handle<DeclarativeEnvironemnt> self = this;
    assert(!this->HasBinding(key));
    Handle<Entry> ent = new Entry(true, canDelete);
    self->entries->Put(key, ent);
}

void DeclarativeEnvironemnt::CreateImmutableBinding(const Handle<JSString>& key, bool strict) {
    Handle<DeclarativeEnvironemnt> self = this;
    assert(!this->HasBinding(key));
    Handle<Entry> ent = new Entry(false, strict);
    self->entries->Put(key, ent);
}

void DeclarativeEnvironemnt::InitializeBinding(const Handle<JSString>& key, const Handle<JSValue>& val) {
    Handle<Entry> entry = this->entries->Get(key);
    assert(entry && !entry->initialized);
    entry->value(val);
    entry->initialized = true;
}

void DeclarativeEnvironemnt::SetMutableBinding(const Handle<JSString>& key, const Handle<JSValue>& val, bool strict) {
    Handle<DeclarativeEnvironemnt> self = this;
    Handle<Entry> entry = this->entries->Get(key);
    if (!entry) {
        if (strict) {
            Exceptions::ThrowReferenceError(key);
        }
        self->CreateMutableBinding(key, true);
        self->InitializeBinding(key, val);
        return;
    }
    if (entry->strict())strict = true;
    if (!entry->initialized) {
        Exceptions::ThrowReferenceError(key);
    }
    if (entry->mutable_) {
        entry->value(val);
    } else {
        if (strict)
            Exceptions::ThrowTypeError("Attempt to modify a const binding");
    }
}

Handle<JSValue> DeclarativeEnvironemnt::GetBindingValue(const Handle<JSString>& key, bool strict) {
    Handle<Entry> entry = this->entries->Get(key);
    assert(entry);
    if (!entry->initialized) {
        Exceptions::ThrowReferenceError(key);
    }
    return entry->value();
}

bool DeclarativeEnvironemnt::DeleteBinding(const Handle<JSString>& key) {
    Handle<DeclarativeEnvironemnt> self = this;
    Handle<Entry> entry = this->entries->Get(key);
    assert(entry);
    if (!entry->canDelete()) {
        return false;
    }
    self->entries->Remove(key);
    return true;
}

bool DeclarativeEnvironemnt::HasThisBinding() {
    return false;
}

bool DeclarativeEnvironemnt::HasSuperBinding() {
    return false;
}

Handle<JSObject> DeclarativeEnvironemnt::WithBaseObject() {
    return nullptr;
}

void DeclarativeEnvironemnt::IterateField(const FieldIterator& iter) {
    Environment::IterateField(iter);
    iter(&this->entries);
}

/* ObjectEnvironment */
ObjectEnvironment::ObjectEnvironment(const Handle<JSObject>& obj, const Handle<Environment>& outer, bool with) :Environment(outer), with(with) {
    this->WriteBarrier(&this->bindingObject_, obj);
}

bool ObjectEnvironment::HasBinding(const Handle<JSString>& key) {
    Handle<JSObject> bindings = this->bindingObject_;
    bool foundBinding = bindings->HasProperty(key);
    if (!foundBinding) return false;
    if (!with) return true;
    Handle<JSValue> unscopables = Objects::Get(bindings, JSSymbol::Unscopables());
    if (unscopables->GetType() != JSValue::Type::kObject) return true;
    bool blocked = Conversion::ToBooleanValue(Objects::Get(unscopables.CastTo<JSObject>(), key));
    return !blocked;
}

void ObjectEnvironment::CreateMutableBinding(const Handle<JSString>& N, bool D) {
    Objects::DefinePropertyOrThrow(bindingObject_, N, PropertyDescriptor{
        { nullptr },
        nullopt,
        nullopt,
        true,
        true,
        D
    });
}

void ObjectEnvironment::CreateImmutableBinding(const Handle<JSString>& key, bool strict) {
    assert(0);
}

void ObjectEnvironment::InitializeBinding(const Handle<JSString>& key, const Handle<JSValue>& val) {
    SetMutableBinding(key, val, false);
}

void ObjectEnvironment::SetMutableBinding(const Handle<JSString>& N, const Handle<JSValue>& V, bool S) {
    Objects::Set(bindingObject_, N, V, S);
}

Handle<JSValue> ObjectEnvironment::GetBindingValue(const Handle<JSString>& N, bool S) {
    Handle<JSObject> bindings = this->bindingObject_;
    bool value = bindingObject_->HasProperty(N);
    if (!value) {
        if (!S)
            return nullptr;
        else
            Exceptions::ThrowReferenceError(N);
    }
    return Objects::Get(bindings, N);
}

bool ObjectEnvironment::DeleteBinding(const Handle<JSString>& key) {
    return this->bindingObject_->Delete(key);
}

bool ObjectEnvironment::HasThisBinding() {
    return false;
}

bool ObjectEnvironment::HasSuperBinding() {
    return false;
}

Handle<JSObject> ObjectEnvironment::WithBaseObject() {
    if (with) return bindingObject_;
    return nullptr;
}

void ObjectEnvironment::IterateField(const FieldIterator& iter) {
    Environment::IterateField(iter);
    iter(&this->bindingObject_);
}

/* FunctionEnvironment */
FunctionEnvironment::FunctionEnvironment(const Handle<ESFunction>& F, const Handle<JSObject>& newTarget):DeclarativeEnvironemnt(F->environment()) {
    this->functionObject(F);
    if (F->thisMode() == ESFunction::ThisMode::kLexical) {
        this->thisBindingStatus(ThisBindingStatus::kLexical);
    } else {
        this->thisBindingStatus(ThisBindingStatus::kUninitialized);
    }
    this->homeObject(F->homeObject());
    this->newTarget(newTarget);
}

void FunctionEnvironment::BindThisValue(const Handle<JSValue>& val) {
    if (this->thisBindingStatus() == ThisBindingStatus::kInitialized) {
        Exceptions::ThrowReferenceError(JSString::New("this"));
    }
    this->thisValue(val);
    this->thisBindingStatus(ThisBindingStatus::kInitialized);
}

bool FunctionEnvironment::HasThisBinding() {
    return this->thisBindingStatus() != ThisBindingStatus::kLexical;
}

bool FunctionEnvironment::HasSuperBinding() {
    return this->thisBindingStatus() != ThisBindingStatus::kLexical && this->homeObject();
}

Handle<JSValue> FunctionEnvironment::GetThisBinding() {
    if (this->thisBindingStatus() == ThisBindingStatus::kUninitialized) {
        Exceptions::ThrowReferenceError(JSString::New("this"));
    }
    return this->thisValue();
}

void FunctionEnvironment::IterateField(const FieldIterator& iter) {
    DeclarativeEnvironemnt::IterateField(iter);
    iter(&this->thisValue_);
    iter(&this->functionObject_);
    iter(&this->homeObject_);
    iter(&this->newTarget_);
}

/* GlobalEnvironment */
GlobalEnvironment::GlobalEnvironment(const Handle<JSObject>& obj) :Environment(nullptr) {
    NoGC _;
    this->WriteBarrier(&this->declRecord_, new DeclarativeEnvironemnt(nullptr));
    this->WriteBarrier(&this->objectRecord_, new ObjectEnvironment(obj, nullptr));
}

bool GlobalEnvironment::HasBinding(const Handle<JSString>& key) {
    Handle<GlobalEnvironment> self = this;
    if (this->declRecord_->HasBinding(key)) {
        return true;
    }
    return self->objectRecord_->HasBinding(key);
}

void GlobalEnvironment::CreateMutableBinding(const Handle<JSString>& N, bool D) {
    Handle<GlobalEnvironment> self = this;
    if (this->declRecord_->HasBinding(N)) {
        Exceptions::ThrowTypeError("There is already a binding with same name");
    }
    self->declRecord_->CreateMutableBinding(N, D);
}

void GlobalEnvironment::CreateImmutableBinding(const Handle<JSString>& N, bool S) {
    Handle<GlobalEnvironment> self = this;
    if (this->declRecord_->HasBinding(N)) {
        Exceptions::ThrowTypeError("There is already a binding with same name");
    }
    self->declRecord_->CreateImmutableBinding(N, S);
}

void GlobalEnvironment::InitializeBinding(const Handle<JSString>& N, const Handle<JSValue>& V) {
    Handle<GlobalEnvironment> self = this;
    if (self->declRecord_->HasBinding(N)) {
        self->declRecord_->InitializeBinding(N, V);
        return;
    }
    assert(self->objectRecord_->HasBinding(N));
    self->objectRecord_->InitializeBinding(N, V);
}

void GlobalEnvironment::SetMutableBinding(const Handle<JSString>& N, const Handle<JSValue>& V, bool S) {
    Handle<GlobalEnvironment> self = this;
    if (self->declRecord_->HasBinding(N)) {
        self->declRecord_->SetMutableBinding(N, V, S);
        return;
    }
    self->objectRecord_->SetMutableBinding(N, V, S);
}

Handle<JSValue> GlobalEnvironment::GetBindingValue(const Handle<JSString>& N, bool S) {
    Handle<GlobalEnvironment> self = this;
    if (self->declRecord_->HasBinding(N)) {
        return self->declRecord_->GetBindingValue(N, S);
    }
    return self->objectRecord_->GetBindingValue(N, S);
}

bool GlobalEnvironment::DeleteBinding(const Handle<JSString>& N) {
    Handle<GlobalEnvironment> self = this;
    if (self->declRecord_->HasBinding(N)) {
        return self->declRecord_->DeleteBinding(N);
    }
    Handle<JSObject> globalObject = self->objectRecord_->bindingObject_;
    bool existingProp = Objects::HasOwnProperty(globalObject, N);
    if (existingProp) {
        bool status = self->objectRecord_->DeleteBinding(N);
        if (status) {
            // Remove from VarNames
        }
        return status;
    }
    return true;
}

bool GlobalEnvironment::HasThisBinding() {
    return true;
}

bool GlobalEnvironment::HasSuperBinding() {
    return false;
}

Handle<JSObject> GlobalEnvironment::WithBaseObject() {
    return nullptr;
}

Handle<JSValue> GlobalEnvironment::GetThisBinding() {
    return this->objectRecord_->bindingObject_;
}

void GlobalEnvironment::IterateField(const FieldIterator& iter) {
    // Environment::IterateField(iter); We know that outer is null
    iter(&this->declRecord_);
    iter(&this->objectRecord_);
}