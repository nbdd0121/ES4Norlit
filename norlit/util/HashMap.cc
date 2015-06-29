#include "HashMap.h"
#include "../gc/Heap.h"

using namespace norlit::gc;
using namespace norlit::util;
using namespace norlit::util::detail;

template<bool kWeak, bool vWeak>
const float HashMapBase<kWeak, vWeak>::DEFAULT_LOAD_FACTOR = 0.75f;

template<bool kWeak, bool vWeak>
struct HashMapBase<kWeak, vWeak>::Entry: public Object {
    uintptr_t hash;
    HashMapBase* base = nullptr;
    Object* key = nullptr;
    Object* value = nullptr;
    Entry* next = nullptr;

    Entry(uintptr_t hashV,
          const Handle<HashMapBase>& base,
          const Handle<Object>& key,
          const Handle<Object>& value,
          const Handle<Entry>& next) {
        hash = hashV;
        this->WriteBarrier(&this->base, base);
        this->WriteBarrier(&this->key, key);
        this->WriteBarrier(&this->value, value);
        this->WriteBarrier(&this->next, next);
    }

    Handle<Object> SetValue(const Handle<Object>& newValue) {
        Handle<Object> oldValue = this->value;
        this->WriteBarrier(&this->value, newValue);
        return oldValue;
    }

    void SetNext(const Handle<Entry>& newNext) {
        this->WriteBarrier(&this->next, newNext);
    }

    virtual void IterateField(const FieldIterator& iter) override final {
        if (kWeak)
            iter(&key, FieldIterator::weak);
        else
            iter(&key);
        if (vWeak)
            iter(&value, FieldIterator::weak);
        else
            iter(&value);
        iter(&next);
    }

    virtual void NotifyWeakReferenceCollected(Object** field) override final {
        if (field == &key)
            base->dirty = true;
    }
};

template<bool kWeak, bool vWeak>
void HashMapBase<kWeak, vWeak>::Clean() {
    // Length(), Get(), Put(), SetNext() will not trigger GC
    Handle<Entry> node;
    Handle<Entry> next;

    for (size_t i = 0, capacity = this->entries->Length(); i < capacity; i++) {
        node = this->entries->Get(i);

        while (node && !node->key) {
            // Remove node from chain
            node = node->next;
            this->entries->Put(i, node);
            this->size--;
        }

        if (!node)
            continue;

        next = node->next;
        while (next) {
            if (!next->key) {
                next = next->next;
                node->SetNext(next);
                this->size--;
                continue;
            }
            node = next;
            next = node->next;
        }
    }
}

template<bool kWeak, bool vWeak>
void HashMapBase<kWeak, vWeak>::Resize(size_t capacity) {
    Handle<HashMapBase> thisPtr = this;
    Handle<Array<Entry>> oldEntries = this->entries;
    Handle<Array<Entry>> newEntries = Array<Entry>::New(capacity);

    for (size_t i = 0; i < oldEntries->Length(); i++) {
        Handle<Entry> ent = oldEntries->Get(i);
        Handle<Entry> next;
        while (ent) {
            next = ent->next;

            size_t bucket = ent->hash % capacity;
            ent->SetNext(newEntries->Get(bucket));
            newEntries->Put(bucket, ent);

            ent = next;
        }
    }

    thisPtr->threshold = static_cast<size_t>(capacity * loadFactor);
    thisPtr->WriteBarrier(&thisPtr->entries, newEntries);
}

template<bool kWeak, bool vWeak>
Handle<typename HashMapBase<kWeak, vWeak>::Entry> HashMapBase<kWeak, vWeak>::GetNode(uintptr_t hash, const Handle<Object>& key) {
    // Get() and Length() will not cause GC
    // we did not use this after Equals()
    Handle<Entry> node = this->entries->Get(hash % this->entries->Length());
    if (!node)
        return nullptr;
    while (node) {
        if (node->hash == hash && (node->key == key || node->key->Equals(key))) {
            return node;
        }
        node = node->next;
    }
    return nullptr;
}

template<bool kWeak, bool vWeak>
Handle<Object> HashMapBase<kWeak, vWeak>::Get(const Handle<Object>& key) {
    if (dirty) Clean();

    // Hash() may cause GC
    Handle<HashMapBase> thisPtr = this;
    uintptr_t hash = Hash(key);
    Handle<Entry> entry = thisPtr->GetNode(hash, key);
    return entry ? entry->value : nullptr;
}

template<bool kWeak, bool vWeak>
Handle<Object> HashMapBase<kWeak, vWeak>::Put(const Handle<Object>& key, const Handle<Object>& value) {
    if (dirty) Clean();

    // Hash() and new Entry() may cause GC
    Handle<HashMapBase> thisPtr = this;

    uintptr_t hash = Hash(key);
    Handle<Entry> entry = thisPtr->GetNode(hash, key);
    if (entry) {
        return entry->SetValue(value);
    } else {
        Handle<Array<Entry>> entries = thisPtr->entries;
        size_t bucket = hash % entries->Length();
        entry = entries->Get(bucket);
        entry = new Entry(hash, thisPtr, key, value, entry);
        entries->Put(bucket, entry);
        if (++thisPtr->size >= thisPtr->threshold) {
            Resize(entries->Length() * 2);
        }
        return nullptr;
    }
}

template<bool kWeak, bool vWeak>
Handle<Object> HashMapBase<kWeak, vWeak>::Remove(const Handle<Object>& key) {
    if (dirty) Clean();

    // Hash() may cause GC
    Handle<HashMapBase> thisPtr = this;
    Handle<Array<Entry>> entries = this->entries;

    uintptr_t hash = Hash(key);
    size_t bucket = hash % entries->Length();
    Handle<Entry> node = entries->Get(bucket);
    if (!node)
        return nullptr;
    if (node->hash == hash && (node->key == key || node->key->Equals(key))) {
        thisPtr->size--;
        entries->Put(bucket, node->next);
        return node->value;
    }

    Handle<Entry> next = node->next;
    while (next) {
        if (next->hash == hash && (next->key == key || next->key->Equals(key))) {
            thisPtr->size--;
            node->SetNext(next->next);
            return next->value;
        }
        node = next;
        next = node->next;
    }
    return nullptr;
}

template<bool kWeak, bool vWeak>
void HashMapBase<kWeak, vWeak>::IterateField(const FieldIterator& iter) {
    iter(&this->entries);
}

template<bool kWeak, bool vWeak>
HashMapBase<kWeak, vWeak>::HashMapBase(size_t capacity, float loadFactor) {
    NoGC _;
    this->loadFactor = loadFactor;
    this->threshold = static_cast<size_t>(capacity * loadFactor);
    this->WriteBarrier(&this->entries, Array<Entry>::New(capacity));
}

namespace norlit {
namespace util {
namespace detail {
template class HashMapBase < true, false > ;
template class HashMapBase < false, false > ;
template class HashMapBase < true, true > ;
template class HashMapBase < false, true > ;
}
}
}