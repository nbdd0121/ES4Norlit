#ifndef NORLIT_UTIL_TAGGEDINTEGER_H
#define NORLIT_UTIL_TAGGEDINTEGER_H

#include "../gc/Object.h"
#include "../gc/Handle.h"

namespace norlit {
namespace util {

class TaggedInteger : public gc::Object {
    static const size_t  MAX_BITS = sizeof(void*) * 8 - 1;
    static const intptr_t MAX_VALUE = (static_cast<intptr_t>(1) << (MAX_BITS - 1)) - 1;
    static const intptr_t MIN_VALUE = -(static_cast<intptr_t>(1) << (MAX_BITS - 1));

    TaggedInteger() = delete;

  public:
    static gc::Handle<TaggedInteger> New(intptr_t number) {
        if (number > MAX_VALUE || number < MIN_VALUE) {
            throw "Out of bound";
        }
        return reinterpret_cast<TaggedInteger*>((number << 1) | 1);
    }

    intptr_t Value() const {
        return reinterpret_cast<intptr_t>(this) >> 1;
    }
};

}
}

#endif