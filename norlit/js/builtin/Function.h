#ifndef NORLIT_JS_BUILTIN_FUNCTION_H
#define NORLIT_JS_BUILTIN_FUNCTION_H

#include "../../gc/Array.h"
#include "../object/JSObject.h"

namespace norlit {
namespace js {
namespace builtin {

class Function {
  public:
    static gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<object::JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<object::JSObject>&);

    static gc::Handle<JSValue> Prototype(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);

    struct prototype {
        static gc::Handle<JSValue> apply(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> bind(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> Symbol_hasInstance(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    };
};

}
}
}

#endif
