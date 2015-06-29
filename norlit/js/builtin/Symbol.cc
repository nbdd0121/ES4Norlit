#include "Builtin.h"

#include "../JSSymbol.h"
#include "../Conversion.h"
#include "../Testing.h"
#include "../Exception.h"

#include "../object/Exotics.h"
#include "../../util/HashMap.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::object;
using namespace norlit::js::builtin;
using namespace norlit::util;

namespace {

Handle<JSSymbol> ToSymbol(const Handle<JSValue>& s, const char* caller) {
    if (Testing::Is<JSSymbol>(s)) {
        return s.CastTo<JSSymbol>();
    } else if (!Testing::Is<JSObject>(s)) {
        Exceptions::ThrowIncompatibleReceiverTypeError(caller);
    } else if (Handle<SymbolObject> sobj = s.ExactCheckedCastTo<SymbolObject>()) {
        return sobj->symbolData();
    } else {
        Exceptions::ThrowIncompatibleReceiverTypeError(caller);
    }
}

HashMap<JSString, JSSymbol, true, true>& GetStringToSymbolHashMap() {
    static HashMap<JSString, JSSymbol, true, true> map;
    return map;
}

HashMap<JSSymbol, JSString, true, true>& GetSymbolToStringHashMap() {
    static HashMap<JSSymbol, JSString, true, true> map;
    return map;
}

}

Handle<JSValue> Symbol::Call(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSString> descString;
    if (args->Length() != 0) {
        Handle<JSValue> desc = args->Get(0);
        if (desc) {
            descString = Conversion::ToString(desc);
        }
    }
    return JSSymbol::New(descString);
}

Handle<JSString> Symbol::SymbolDescriptiveString(const Handle<JSSymbol>& sym) {
    Handle<JSString> desc = sym->Descriptor();
    if (!desc) {
        desc = JSString::New("");
    }
    return JSString::Concat({ JSString::New("Symbol("), desc, JSString::New(")") });
}

Handle<JSValue> Symbol::for_(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSString> key = Conversion::ToString(GetArg(args, 0));
    Handle<JSSymbol> sym = GetStringToSymbolHashMap().Get(key);
    if(sym) {
        return sym;
    }
    sym = JSSymbol::New(key);
    GetStringToSymbolHashMap().Put(key, sym);
    GetSymbolToStringHashMap().Put(sym, key);
    return sym;
}

Handle<JSValue> Symbol::keyFor(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSValue> sym = GetArg(args, 0);
    if (!Testing::Is<JSSymbol>(sym)) {
        Exceptions::ThrowTypeError("Argument of keyFor is not a Symbol");
    }
    return GetSymbolToStringHashMap().Get(sym.CastTo<JSSymbol>());
}


Handle<JSValue> Symbol::prototype::toString(const Handle<JSValue>& s, const Handle<Array<JSValue>>&) {
    return SymbolDescriptiveString(ToSymbol(s, "Symbol.prototype.toString"));
}

Handle<JSValue> Symbol::prototype::valueOf(const Handle<JSValue>& s, const Handle<Array<JSValue>>&) {
    return ToSymbol(s, "Symbol.prototype.valueOf");
}

Handle<JSValue> Symbol::prototype::Symbol_toPrimitive(const Handle<JSValue>& s, const Handle<Array<JSValue>>&) {
    return ToSymbol(s, "Symbol.prototype[Symbol.toPrimitive]");
}