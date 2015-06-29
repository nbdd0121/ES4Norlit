#ifndef NORLIT_UTIL_OPTIONAL_H
#define NORLIT_UTIL_OPTIONAL_H

#include <stdexcept>
#include <type_traits>

namespace norlit {
namespace util {

class NullOpt {};

extern NullOpt nullopt;

template<typename T>
class Optional {
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type data;
    bool engaged;
  public:
    Optional() :engaged{ false } {}
    Optional(NullOpt) : engaged{ false } {}
    Optional(const Optional& opt) :engaged{ opt.engaged } {
        if (engaged) {
            new(&data)T(opt.operator*());
        }
    }
    Optional(Optional&& opt) :engaged{ opt.engaged } {
        if (engaged) {
            new(&data)T(std::move(opt.operator*()));
        }
    }
    Optional(const T& val) :engaged{ true } {
        new(&data)T(val);
    }
    Optional(T&& val) :engaged{ true } {
        new(&data)T(std::move(val));
    }
    ~Optional() {
        if (engaged) {
            reinterpret_cast<T*>(&data)->~T();
        }
    }
    void operator= (NullOpt) {
        this->~Optional();
        engaged = false;
    }
    void operator= (const Optional& opt) {
        this->~Optional();
        engaged = opt.engaged;
        if (engaged) {
            new(&data)T(opt.operator*());
        }
    }
    void operator= (Optional&& opt) {
        this->~Optional();
        engaged = opt.engaged;
        if (engaged) {
            new(&data)T(std::move(opt.operator*()));
        }
    }
    void operator= (const T& val) {
        this->~Optional();
        engaged = true;
        new(&data)T(val);
    }
    void operator= (T&& val) {
        this->~Optional();
        engaged = true;
        new(&data)T(std::move(val));
    }

    explicit operator bool() const {
        return engaged;
    }

    T& operator*() {
        if (engaged) {
            return *reinterpret_cast<T*>(&data);
        } else {
            throw std::runtime_error{"Optional is not engaged"};
        }
    }

    const T& operator*() const {
        if (engaged) {
            return *reinterpret_cast<const T*>(&data);
        } else {
            throw std::runtime_error{ "Optional is not engaged" };
        }
    }

    T* operator->() {
        if (engaged) {
            return reinterpret_cast<T*>(&data);
        } else {
            return nullptr;
        }
    }

    const T* operator->() const {
        if (engaged) {
            return reinterpret_cast<const T*>(&data);
        } else {
            return nullptr;
        }
    }

};

}
}

#endif