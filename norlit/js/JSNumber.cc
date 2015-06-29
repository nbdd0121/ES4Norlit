#include "JSNumber.h"

using namespace norlit::js;
using namespace norlit::gc;

JSNumber::JSNumber(double value) :value(value) {
}

JSValue::Type JSNumber::VirtualGetType() const {
    return Type::kNumber;
}
