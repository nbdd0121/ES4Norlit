#ifndef NORLIT_JS_JSBOOLEAN_H
#define NORLIT_JS_JSBOOLEAN_H

#include "JSPrimitive.h"

namespace norlit {
namespace js {

class JSBoolean final : public JSPrimitive {
    /*
    * Tagged Boolean Representation
    *
    * Byte 0: 0000v100
    *             ^
    *       boolean value
    */
  private:
    JSBoolean() = delete;
  public:
    // No need of Handle. JSBoolean are never actually created
    static JSBoolean* New(bool value);

    bool Value() const;
};

inline JSBoolean* JSBoolean::New(bool value) {
    return reinterpret_cast<JSBoolean*>(value << 3 | 4);
}

inline bool JSBoolean::Value() const {
    return !!(reinterpret_cast<uintptr_t>(this) & 8);
}

}
}

#endif
