#ifndef NORLIT_UTIL_VALUEARRAYLIST_H
#define NORLIT_UTIL_VALUEARRAYLIST_H

#include "../gc/Array.h"
#include "../gc/Heap.h"

#include <algorithm>

namespace norlit {
namespace util {

template<typename T>
class ValueArrayList: public gc::Object {
    static const size_t DEFAULT_INITIAL_CAPACITY = 10;
    gc::ValueArray<T>* elements = nullptr;
    size_t size = 0;

    void EnsureCapacity(size_t capacity) {
        using namespace gc;

        size_t oldCapacity = this->elements->Length();
        if (oldCapacity >= capacity)
            return;
        size_t newCapacity = oldCapacity * 3 / 2;
        capacity = std::max(capacity, newCapacity);

        Handle<ValueArrayList> thisPtr = this;
        Handle<ValueArray<T>> newArray = ValueArray<T>::New(capacity);
        Handle<ValueArray<T>> oldArray = thisPtr->elements;

        for (size_t i = 0, size = thisPtr->size; i < size; i++) {
            newArray->At(i) = oldArray->At(i);
        }

        thisPtr->WriteBarrier(&thisPtr->elements, newArray);
    }

    virtual void IterateField(const gc::FieldIterator& iter) override {
        iter(&elements);
    }
  public:
    size_t Size() {
        return size;
    }

    T Get(size_t index) {
        if (index > size) {
            throw "IndexOutOfBound";
        }
        return elements->At(index);
    }

    T Set(size_t index, const T& value) {
        if (index > size) {
            throw "IndexOutOfBound";
        }
        T ret = elements->At(index);
        elements->At(index) = value;
        return ret;
    }

    void Add(const T& value) {
        gc::Handle<ValueArrayList> self = this;
        self->EnsureCapacity(self->size + 1);

        size_t index = self->size++;
        self->elements->At(index) = value;
    }

    ValueArrayList(size_t capacity = DEFAULT_INITIAL_CAPACITY) {
        gc::NoGC _;
        WriteBarrier(&this->elements, gc::ValueArray<T>::New(capacity));
    }

    gc::Handle<gc::ValueArray<T>> ToArray() {
        gc::Handle<ValueArrayList> thisPtr = this;
        size_t size = Size();
        gc::Handle<gc::ValueArray<T>> arr = gc::ValueArray<T>::New(size);
        for (size_t i = 0; i < size; i++) {
            arr->At(i) = thisPtr->Get(i);
        }
        return arr;
    }
};


}
}

#endif