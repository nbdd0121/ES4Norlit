#include "../all.h"

#include "Builtin.h"

#include "../vm/Context.h"
#include "../object/Exotics.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::builtin;

Handle<JSValue> String::Call(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSValue> value = args->Length() > 0 ? args->Get(0) : JSString::New("");
    if (Handle<JSSymbol> sym = Testing::CastIf<JSSymbol>(value)) {
        return Symbol::SymbolDescriptiveString(sym);
    }
    return Conversion::ToString(value);
}

Handle<JSObject> String::Construct(const Handle<Array<JSValue>>& args, const Handle<JSObject>& target) {
    Handle<JSString> s = args->Length() > 0 ? Conversion::ToString(args->Get(0)) : JSString::New("");
    return Objects::StringCreate(s, Objects::GetPrototypeFromConstructor(target, &Realm::StringPrototype));
}
