#ifndef NORLIT_UTIL_ARRAYS_H
#define NORLIT_UTIL_ARRAYS_H

#include "../gc/Array.h"
#include <initializer_list>

namespace norlit {
namespace util {

class Arrays {
  private:
    template<size_t index, typename T>
    static void PutItem(const gc::Handle<gc::Array<T>>& arr) {}

    template<size_t index, typename T, typename... Args>
    static void PutItem(const gc::Handle<gc::Array<T>>& arr, const gc::Handle<T>& val, Args... args) {
        arr->Put(index, val);
        PutItem<index + 1>(arr, args...);
    }
  public:
    template<typename T, typename ...Args>
    static gc::Handle<gc::Array<T>> ToArray(Args... args) {
        gc::Handle<gc::Array<T>> arr = gc::Array<T>::New(sizeof...(Args));
        PutItem<0, T>(arr, args...);
        return arr;
    }

    template<typename T>
    static gc::Handle<gc::Array<T>> Concat(const gc::Handle<gc::Array<T>>& a1, const gc::Handle<gc::Array<T>>& a2) {
        size_t l1 = a1->Length(), l2 = a2->Length();
        gc::Handle<gc::Array<T>> arr = gc::Array<T>::New(l1 + l2);
        for (size_t i = 0; i < l1; i++) {
            arr->Put(i, a1->Get(i));
        }
        for (size_t i = 0; i < l2; i++) {
            arr->Put(l1 + i, a2->Get(i));
        }
        return arr;
    }
};


}
}

#endif