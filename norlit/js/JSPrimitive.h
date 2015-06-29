#ifndef NORLIT_JS_JSPRIMITIVE_H
#define NORLIT_JS_JSPRIMITIVE_H

#include "JSValue.h"

namespace norlit {
namespace js {

class JSPrimitive : public JSValue {

};

class JSNull : public JSPrimitive {
  public:
    static JSNull* New();
};

class JSUndefined : public JSPrimitive {
  public:
    static JSUndefined* New();
};

inline JSNull* JSNull::New() {
    return reinterpret_cast<JSNull*>(6);
}

inline JSUndefined* JSUndefined::New() {
    return nullptr;
}

}
}

#endif