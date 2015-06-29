#include "../all.h"

#include "Builtin.h"

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::object;
using namespace norlit::js::builtin;

Handle<JSValue> builtin::isFinite(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    return JSBoolean::New(std::isfinite(Conversion::ToNumber(GetArg(args, 0))->Value()));
}

Handle<JSValue> builtin::isNaN(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    return JSBoolean::New(std::isnan(Conversion::ToNumber(GetArg(args, 0))->Value()));
}

Handle<JSValue> builtin::parseFloat(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSString> str = Conversion::ToString(GetArg(args, 0))->TrimLeft();
    size_t ptr = 0;
    return Conversion::ScanFloat(str, ptr);
}

Handle<JSValue> builtin::parseInt(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    Handle<JSString> str = Conversion::ToString(GetArg(args, 0))->TrimLeft();
    size_t ptr = 0, len=str->Length();
    if (len == 0) return JSNumber::NaN();
    bool sign = false;
    if (str->At(0) == '+') {
        ptr++;
    } else if(str->At(0)=='-') {
        ptr++;
        sign = true;
    }
    int32_t R = Conversion::ToInt32(Conversion::ToNumber(GetArg(args, 1)));
    bool stripPrefix = true;
    if (R != 0) {
        if (R<2 || R>36) return JSNumber::NaN();
        if (R != 16) stripPrefix = false;
    } else {
        R = 10;
    }
    if (stripPrefix) {
        if (ptr+1 < len) {
            if (str->At(ptr) == '0' && (str->At(ptr + 1) == 'x' || str->At(ptr + 1) == 'X')) {
                ptr += 2;
                R = 16;
            }
        }
    }

    Handle<JSNumber> ret = Conversion::ScanInt(str, ptr, R);
    if (sign)
        return ret->Negate();
    else
        return ret;
}