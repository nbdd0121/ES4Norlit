#include "Builtin.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::builtin;

Handle<JSValue> builtin::GetArg(const Handle<Array<JSValue>>& args, size_t index) {
    if (args->Length() <= index) {
        return nullptr;
    }
    return args->Get(index);
}

Handle<Array<JSValue>> builtin::GetRestArg(const Handle<Array<JSValue>>& args, size_t index) {
    if (args->Length() <= index) {
        return Array<JSValue>::New(0);
    }
    size_t size = args->Length() - index;;
    Handle<Array<JSValue>> ret = Array<JSValue>::New(size);
    for (size_t i = 0; i < size; i++) {
        ret->Put(i, args->Get(index + i));
    }
    return ret;
}