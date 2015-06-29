#ifndef NORLIT_JS_ITERATORS_H
#define NORLIT_JS_ITERATORS_H

#include "object/JSOrdinaryObject.h"

#include "JSString.h"

#include "../util/Arrays.h"

namespace norlit {
namespace js {

struct Iterators {
    static gc::Handle<object::JSObject> GetIterator(const gc::Handle<JSValue>&, const gc::Handle<object::JSObject>&);
    static gc::Handle<object::JSObject> GetIterator(const gc::Handle<JSValue>&);

    static gc::Handle<object::JSObject> IteratorNext(const gc::Handle<object::JSObject>&, const gc::Handle<JSValue>&);
    static gc::Handle<object::JSObject> IteratorNext(const gc::Handle<object::JSObject>&);

    static bool IteratorComplete(const gc::Handle<object::JSObject>&);
    static gc::Handle<JSValue> IteratorValue(const gc::Handle<object::JSObject>&);
    static gc::Handle<object::JSObject> IteratorStep(const gc::Handle<object::JSObject>&);

    static gc::Handle<object::JSObject> CreateIterResultObject(const gc::Handle<JSValue>&, bool);
};

}
}

#endif
