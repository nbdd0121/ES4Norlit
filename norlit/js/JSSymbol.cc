#include "JSSymbol.h"

using namespace norlit::gc;
using namespace norlit::js;

void JSSymbol::IterateField(const FieldIterator& iter) {
    iter(&desc);
}

JSValue::Type JSSymbol::VirtualGetType() const {
    return Type::kSymbol;
}

JSSymbol::JSSymbol() {}

JSSymbol::JSSymbol(const Handle<JSString>& desc) {
    this->WriteBarrier(&this->desc, desc);
}

Handle<JSSymbol> JSSymbol::HasInstance() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.hasInstance");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}

Handle<JSSymbol> JSSymbol::IsConcatSpreadable() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.isConcatSpreadable");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}

Handle<JSSymbol> JSSymbol::Iterator() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.iterator");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}

Handle<JSSymbol> JSSymbol::Match() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.match");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}

Handle<JSSymbol> JSSymbol::Replace() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.replace");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}

Handle<JSSymbol> JSSymbol::Search() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.search");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}

Handle<JSSymbol> JSSymbol::Species() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.species");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}

Handle<JSSymbol> JSSymbol::Split() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.split");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}

Handle<JSSymbol> JSSymbol::ToPrimitive() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.toPrimitive");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}

Handle<JSSymbol> JSSymbol::ToStringTag() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.toStringTag");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}

Handle<JSSymbol> JSSymbol::Unscopables() {
    static Handle<JSSymbol> symbol = [] () {
        Handle<JSString> desc = JSString::New("Symbol.unscopables");
        return new JSSymbol(std::move(desc));
    }();
    return symbol;
}
