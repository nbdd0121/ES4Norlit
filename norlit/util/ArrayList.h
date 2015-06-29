#ifndef NORLIT_UTIL_ARRAYLIST_H
#define NORLIT_UTIL_ARRAYLIST_H

#include "../gc/Array.h"

namespace norlit {
namespace util {
namespace detail {

class ArrayListBase : public gc::Object {
  private:
    gc::Array<Object>* elements = nullptr;
    size_t size = 0;

    void EnsureCapacity(size_t minCapacity);
    virtual void IterateField(const gc::FieldIterator&) override;
  protected:
    static const size_t DEFAULT_INITIAL_CAPACITY = 10;

    void Add(const gc::Handle<gc::Object>& value);
    size_t Size() {
        return size;
    }
    gc::Handle<gc::Object> Get(size_t index) {
        if (index > size) {
            throw "IndexOutOfBound";
        }
        return elements->Get(index);
    }
    gc::Handle<gc::Object> Set(size_t index, const gc::Handle<gc::Object>& value) {
        if (index > size) {
            throw "IndexOutOfBound";
        }
        gc::Handle<gc::Object> ret = elements->Get(index);
        elements->Put(index, value);
        return ret;
    }
    gc::Handle<gc::Object> Remove(size_t index);
    ArrayListBase(size_t);

  public:
    void Clear();
};
}

template<typename T>
class ArrayList : public detail::ArrayListBase {
  public:
    size_t Size() {
        return detail::ArrayListBase::Size();
    }

    gc::Handle<T> Get(size_t index) {
        return detail::ArrayListBase::Get(index).template CastTo<T>();
    }

    gc::Handle<T> GetLast() {
        return Get(Size() - 1);
    }

    gc::Handle<T> Set(size_t index, const gc::Handle<T>& value) {
        return detail::ArrayListBase::Set(index, value).template CastTo<T>();
    }

    gc::Handle<T> Remove(size_t index) {
        return detail::ArrayListBase::Remove(index).template CastTo<T>();
    }

    gc::Handle<T> RemoveLast() {
        return Remove(Size() - 1);
    }

    void Add(const gc::Handle<T>& value) {
        detail::ArrayListBase::Add(value);
    }

    ArrayList(size_t capacity = detail::ArrayListBase::DEFAULT_INITIAL_CAPACITY) :
        detail::ArrayListBase(capacity) {

    }

    gc::Handle<gc::Array<T>> ToArray() {
        gc::Handle<ArrayList> thisPtr = this;
        size_t size = Size();
        gc::Handle<gc::Array<T>> arr = gc::Array<T>::New(size);
        gc::Handle<T> tmp;
        for (size_t i = 0; i < size; i++) {
            tmp = thisPtr->Get(i);
            arr->Put(i, tmp);
        }
        return arr;
    }
};


}
}

#endif