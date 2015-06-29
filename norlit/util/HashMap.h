#ifndef NORLIT_UTIL_HASHMAP_H
#define NORLIT_UTIL_HASHMAP_H

#include "../gc/Array.h"

namespace norlit {
namespace util {
namespace detail {
template<bool kWeak, bool vWeak>
class HashMapBase : public gc::Object {
  private:
    struct Entry;

    static uintptr_t Hash(const gc::Handle<gc::Object>& key) {
        if (key->IsTagged()) {
            return reinterpret_cast<uintptr_t>(static_cast<gc::Object*>(key));
        } else {
            return key->HashCode();
        }
    }

    gc::Array<Entry>* entries = nullptr;
    size_t size = 0;
    size_t threshold;
    float loadFactor;
    // Used by weak hashmap
    bool dirty = false;

    void Clean();
    void Resize(size_t capacity);
    gc::Handle<Entry> GetNode(uintptr_t, const gc::Handle<gc::Object>&);
    virtual void IterateField(const gc::FieldIterator&) override;
  protected:
    static const size_t DEFAULT_INITIAL_CAPACITY = 16;
    static const float DEFAULT_LOAD_FACTOR;

    gc::Handle<gc::Object> Get(const gc::Handle<gc::Object>& key);
    gc::Handle<gc::Object> Put(const gc::Handle<gc::Object>& key, const gc::Handle<gc::Object>& value);
    gc::Handle<gc::Object> Remove(const gc::Handle<gc::Object>& key);

    HashMapBase(size_t, float);
};
}

template<typename K, typename V, bool kWeak = false, bool vWeak = false>
class HashMap: public detail::HashMapBase<kWeak, vWeak> {
  public:
    gc::Handle<V> Get(const gc::Handle<K>& key) {
        return (V*)detail::HashMapBase<kWeak, vWeak>::Get((gc::Object*)key);
    }

    gc::Handle<V> Put(const gc::Handle<K>& key, const gc::Handle<V>& value) {
        return (V*)detail::HashMapBase<kWeak, vWeak>::Put((gc::Object*)key, (gc::Object*)value);
    }

    gc::Handle<V> Remove(const gc::Handle<K>& key) {
        return (V*)detail::HashMapBase<kWeak, vWeak>::Remove((gc::Object*)key);
    }

    HashMap(size_t capacity = detail::HashMapBase<kWeak, vWeak>::DEFAULT_INITIAL_CAPACITY,
            float loadFactor = detail::HashMapBase<kWeak, vWeak>::DEFAULT_LOAD_FACTOR) :
        detail::HashMapBase<kWeak, vWeak>(capacity, loadFactor) {

    }
};

}
}

#endif