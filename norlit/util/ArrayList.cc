#include "ArrayList.h"

#include "../gc/Heap.h"

#include <algorithm>

using namespace norlit::gc;
using namespace norlit::util::detail;

void ArrayListBase::EnsureCapacity(size_t capacity) {
    size_t oldCapacity = this->elements->Length();
    if (oldCapacity >= capacity)
        return;
    size_t newCapacity = oldCapacity * 3 / 2;
    capacity = std::max(capacity, newCapacity);

    Handle<ArrayListBase> thisPtr = this;
    Handle<Array<Object>> newArray = Array<Object>::New(capacity);
    Handle<Array<Object>> oldArray = thisPtr->elements;

    for (size_t i = 0, size = thisPtr->size; i < size; i++) {
        newArray->Put(i, oldArray->Get(i));
    }

    thisPtr->WriteBarrier(&thisPtr->elements, newArray);
}

void ArrayListBase::IterateField(const FieldIterator& iter) {
    iter(&elements);
}

void ArrayListBase::Add(const Handle<Object>& value) {
    Handle<ArrayListBase> thisPtr = this;
    this->EnsureCapacity(this->size + 1);

    size_t index = thisPtr->size++;
    thisPtr->elements->Put(index, value);
}

Handle<Object> ArrayListBase::Remove(size_t index) {
    if (index > size) {
        throw "IndexOutOfBound";
    }
    Handle<Object> ret = this->elements->Get(index);
    for (size_t size = this->size; index < size - 1; index++) {
        this->elements->Put(index, this->elements->Get(index + 1));
    }
    this->elements->Put(index, nullptr);
    this->size--;
    return ret;
}

ArrayListBase::ArrayListBase(size_t capacity) {
    NoGC _;
    WriteBarrier(&this->elements, Array<Object>::New(capacity));
}

void ArrayListBase::Clear() {
    for (size_t i = 0; i < this->size; i++) {
        this->elements->Put(i, nullptr);
    }
    this->size = 0;
}