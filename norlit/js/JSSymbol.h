#ifndef NORLIT_JS_JSSYMBOL_H
#define NORLIT_JS_JSSYMBOL_H

#include "JSPropertyKey.h"
#include "JSString.h"
#include "../gc/Handle.h"

namespace norlit {
namespace js {

class JSSymbol final : public JSPropertyKey {
  private:
    JSString* desc = nullptr;

    JSSymbol();
    JSSymbol(const gc::Handle<JSString>&);

    virtual void IterateField(const gc::FieldIterator& iter) override final;
    virtual Type VirtualGetType() const override final;
  public:
    static gc::Handle<JSSymbol> New();
    static gc::Handle<JSSymbol> New(const gc::Handle<JSString>&);
    static gc::Handle<JSSymbol> HasInstance();
    static gc::Handle<JSSymbol> IsConcatSpreadable();
    static gc::Handle<JSSymbol> Iterator();
    static gc::Handle<JSSymbol> Match();
    static gc::Handle<JSSymbol> Replace();
    static gc::Handle<JSSymbol> Search();
    static gc::Handle<JSSymbol> Species();
    static gc::Handle<JSSymbol> Split();
    static gc::Handle<JSSymbol> ToPrimitive();
    static gc::Handle<JSSymbol> ToStringTag();
    static gc::Handle<JSSymbol> Unscopables();

    gc::Handle<JSString> Descriptor() const;
};

inline gc::Handle<JSString> JSSymbol::Descriptor() const {
    return this->desc;
}

inline gc::Handle<JSSymbol> JSSymbol::New() {
    return new JSSymbol();
}

inline gc::Handle<JSSymbol> JSSymbol::New(const gc::Handle<JSString>& desc) {
    return new JSSymbol(desc);
}

}
}

#endif
