#include "all.h"

#include "JSBoolean.h"
#include "JSString.h"
#include "JSSymbol.h"
#include "object/JSObject.h"
#include "object/Exotics.h"

#include "grammar/Scanner.h"

#include "../util/Arrays.h"
#include "../util/Double2String.h"

#include "vm/Context.h"
#include "vm/Realm.h"

#include <limits>

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::grammar;
using namespace norlit::util;

namespace {

void ScanDecimal(const Handle<JSString>& val, size_t& ptr, uint64_t& ret, int16_t& length, int16_t* overflow, uint8_t base) {
    size_t len = val->Length();
    uint64_t overflowLimit = 0x7FFFFFFFFFFFFFFFULL / base;
    bool overflowFlag = false;
    while (ptr<len) {
        char16_t ch = val->At(ptr);
        int val = Scanner::GetDigit(ch);
        if (val>=0 && val < base) {
            if (!overflowFlag) {
                if (ret < overflowLimit) {
                    ret = ret * base + val;
                    length++;
                } else {
                    overflowFlag = true;
                    if (val >= base / 2) {
                        ret++;
                    }
                    if (overflow) {
                        (*overflow)++;
                    }
                }
            } else if (overflow) {
                (*overflow)++;
            }
            ptr++;
        } else {
            return;
        }
    }
}

Handle<JSNumber> ScanNumber(const Handle<JSString>& val) {
    size_t len = val->Length();
    if (len == 0) return JSNumber::Zero();

    if (val->At(0)=='0') {
        if (len == 1) return JSNumber::Zero();

        int base;
        switch (val->At(1)) {
            case 'b':
            case 'B':
                base = 2;
                break;
            case 'o':
            case 'O':
                base = 8;
                break;
            case 'x':
            case 'X':
                base = 16;
                break;
            default:
                goto decimal;
        }

        size_t ptr = 2;
        Handle<JSNumber> number = Conversion::ScanInt(val, ptr, base);

        if (ptr != len) return JSNumber::NaN();
        return number;
    }
decimal:
    size_t ptr = 0;
    Handle<JSNumber> number = Conversion::ScanFloat(val, ptr);
    if (ptr != len) return JSNumber::NaN();
    return number;
}

}

Handle<JSPrimitive> Conversion::ToPrimitive(const Handle<JSValue>& val, JSValue::Type preferredType) {
    if (Testing::Is<JSPrimitive>(val)) {
        return val.CastTo<JSPrimitive>();
    }

    Handle<JSObject> input = val.CastTo<JSObject>();
    Handle<JSObject> exoticToPrim = Objects::GetMethod(input, JSSymbol::ToPrimitive());
    if (exoticToPrim) {
        Handle<JSString> hint;
        switch (preferredType) {
            case JSValue::Type::kNumber:
                hint = JSString::New("number");
                break;
            case JSValue::Type::kString:
                hint = JSString::New("string");
                break;
            default:
                hint = JSString::New("default");
                break;
        }
        Handle<Array<JSValue>> array = Arrays::ToArray<JSValue>(hint.CastTo<JSValue>());
        Handle<JSValue> result = Objects::Call(exoticToPrim, input, array);
        if (Testing::Is<JSPrimitive>(result)) {
            return result.CastTo<JSPrimitive>();
        }
        Exceptions::ThrowTypeError("Cannot convert the object to a primitive value");
    }

    if (preferredType == JSValue::Type::kUndefined) {
        preferredType = JSValue::Type::kNumber;
    }

    return OrdinaryToPrimitive(input, preferredType);
}

Handle<JSPrimitive> Conversion::OrdinaryToPrimitive(const Handle<JSObject>& O, JSValue::Type preferredType) {
    const char *first, *second;
    if (preferredType == JSValue::Type::kString) {
        first = "toString";
        second = "valueOf";
    } else {
        first = "valueOf";
        second = "toString";
    }

    Handle<JSValue> method = Objects::Get(O, first);
    if (Testing::IsCallable(method)) {
        Handle<JSObject> methodAsObj = method.CastTo<JSObject>();
        Handle<JSValue> result = Objects::Call(methodAsObj, O);
        if (Testing::Is<JSPrimitive>(result)) {
            return result.CastTo<JSPrimitive>();
        }
    }
    method = Objects::Get(O, second);
    if (Testing::IsCallable(method)) {
        Handle<JSObject> methodAsObj = method.CastTo<JSObject>();
        Handle<JSValue> result = Objects::Call(methodAsObj, O);
        if (Testing::Is<JSPrimitive>(result)) {
            return result.CastTo<JSPrimitive>();
        }
    }

    Exceptions::ThrowTypeError("Cannot convert the object to a primitive value");
}

Handle<JSBoolean> Conversion::ToBoolean(const Handle<JSValue>& val) {
    switch (val->GetType()) {
        case JSValue::Type::kUndefined:
        case JSValue::Type::kNull:
            return JSBoolean::New(false);
        case JSValue::Type::kBoolean:
            return val.CastTo<JSBoolean>();
        case JSValue::Type::kNumber: {
            double num = val.CastTo<JSNumber>()->Value();
            return JSBoolean::New(num != 0 && !std::isnan(num));
        }
        case JSValue::Type::kString:
            return JSBoolean::New(val.CastTo<JSString>()->Length() != 0);
        default:
            return JSBoolean::New(true);
    }
}

bool Conversion::ToBooleanValue(const Handle<JSValue>& val) {
    return ToBoolean(val)->Value();
}

Handle<JSNumber> Conversion::ToNumber(const Handle<JSValue>& val) {
    switch (val->GetType()) {
        case JSValue::Type::kUndefined:
            return JSNumber::New(std::numeric_limits<double>::quiet_NaN());
        case JSValue::Type::kNull:
            return JSNumber::New(0);
        case JSValue::Type::kBoolean:
            return JSNumber::New(val.CastTo<JSBoolean>()->Value());
        case JSValue::Type::kNumber:
            return val.CastTo<JSNumber>();
        case JSValue::Type::kString:
            return ScanNumber(val.CastTo<JSString>()->Trim());
        case JSValue::Type::kSymbol:
            Exceptions::ThrowTypeError("Cannot convert a Symbol to a number");
        default:
            return ToNumber(ToPrimitive(val, JSValue::Type::kNumber));
    }
}

double Conversion::ToNumberValue(const Handle<JSValue>& val) {
    return ToNumber(val)->Value();
}

Handle<JSString> Conversion::ToString(const Handle<JSValue>& val) {
    switch (val->GetType()) {
        case JSValue::Type::kUndefined:
            return JSString::New("undefined");
        case JSValue::Type::kNull:
            return JSString::New("null");
        case JSValue::Type::kBoolean:
            return JSString::New(val.CastTo<JSBoolean>()->Value() ? "true" : "false");
        case JSValue::Type::kNumber: {
            double value = val.CastTo<JSNumber>()->Value();
            if (value == 0) {
                return JSString::New("0");
            } else if (std::isfinite(value)) {
                std::string builder;

                if (value < 0) {
                    value = -value;
                    builder += '-';
                }

                uint64_t s;
                int16_t n;
                uint8_t k;
                DesembleDouble(value, s, n, k);

                auto appendToBuilder = [] (std::string& builder, uint64_t num, int length) {
                    auto len = builder.length();
                    builder.resize(len + length);
                    for (int i = length - 1; i >= 0; i--) {
                        builder[len + i] = '0' + num % 10;
                        num /= 10;
                    }
                    return num;
                };

                //printf("%llu %d-%d\n", s, n, k);
                if (k <= n && n <= 21) {
                    appendToBuilder(builder, s, k);
                    for (int i = 0; i < n - k; i++) {
                        builder += '0';
                    }
                } else if (0 < n&&n <= 21) {
                    std::string decimals;
                    s = appendToBuilder(decimals, s, k - n);
                    appendToBuilder(builder, s, n);
                    builder += '.';
                    builder += decimals;
                } else if (-6 < n && n <= 0) {
                    builder += '0';
                    builder += '.';
                    for (int i = 0; i < n; i++) {
                        builder += '0';
                    }
                    appendToBuilder(builder, s, k);
                } else if (k == 1) {
                    builder += static_cast<char>(s)+'0';
                    builder += 'e';
                    n--;
                    if (n > 0) {
                        builder += '+';
                    } else {
                        n = -n;
                        builder += '-';
                    }
                    appendToBuilder(builder, n, CountDigits(n));
                } else {
                    std::string decimals;
                    s = appendToBuilder(decimals, s, k - 1);
                    builder += static_cast<char>(s)+'0';
                    builder += '.';
                    builder += decimals;

                    builder += 'e';
                    n--;
                    if (n > 0) {
                        builder += '+';
                    } else {
                        n = -n;
                        builder += '-';
                    }
                    appendToBuilder(builder, n, CountDigits(n));
                }
                return JSString::New(builder.c_str());
            } else if (std::isnan(value)) {
                return JSString::New("NaN");
            } else {
                if (value > 0) {
                    return JSString::New("Infinity");
                } else {
                    return JSString::New("-Infinity");
                }
            }
        }
        case JSValue::Type::kString:
            return val.CastTo<JSString>();
        case JSValue::Type::kSymbol:
            Exceptions::ThrowTypeError("Cannot convert a Symbol to a string");
        default:
            return ToString(ToPrimitive(val, JSValue::Type::kString));
    }
}

Handle<JSObject> Conversion::ToObject(const Handle<JSValue>& val) {
    switch (val->GetType()) {
        case JSValue::Type::kObject:
            return val.CastTo<JSObject>();
        case JSValue::Type::kBoolean: {
            Handle<BooleanObject> obj = new BooleanObject(Context::CurrentRealm()->BooleanPrototype());
            obj->booleanData(val.CastTo<JSBoolean>()->Value());
            return obj;
        }
        case JSValue::Type::kNumber: {
            Handle<NumberObject> obj = new NumberObject(Context::CurrentRealm()->NumberPrototype());
            obj->numberData(val.CastTo<JSNumber>()->Value());
            return obj;
        }
        case JSValue::Type::kString:
            assert(0);
        case JSValue::Type::kSymbol: {
            Handle<SymbolObject> obj = new SymbolObject(Context::CurrentRealm()->SymbolPrototype());
            obj->symbolData(val.CastTo<JSSymbol>());
            return obj;
        }
        default:
            Exceptions::ThrowTypeError("TypeError: Cannot convert null or undefined to object");
    }
}

Handle<JSPropertyKey> Conversion::ToPropertyKey(const Handle<JSValue>& val) {
    if (val->GetType() == JSValue::Type::kSymbol)
        return val.CastTo<JSPropertyKey>();
    else
        return ToString(val);
}

int64_t Conversion::ToIntegerValue(const Handle<JSNumber>& num) {
    double val = num->Value();
    if (std::isnan(val)) {
        return 0;
    }
    if (std::isinf(val)) {
        throw "internal error: infinity should not be feeded to ToIntegerValue";
    }
    return static_cast<int64_t>(val);
}

int32_t Conversion::ToInt32(const Handle<JSNumber>& num) {
    double val = num->Value();
    if (std::isnan(val) || std::isinf(val)) {
        return 0;
    }
    return static_cast<int32_t>(static_cast<int64_t>(val));
}

uint32_t Conversion::ToUInt32(const Handle<JSNumber>& num) {
    return static_cast<uint32_t>(ToInt32(num));
}

int64_t Conversion::ToIntegerIndex(const Handle<JSString>& str) {
    size_t ptr = 0;
    Handle<JSNumber> num = ScanInt(str, ptr, 10);
    double val = num->Value();
    int64_t ret = static_cast<int64_t>(val);
    if (ret != val || ret < 0 || ret>0x1fffffffffffff) {
        return -1;
    }
    return ret;
}

int64_t Conversion::ToIntegerIndex(const Handle<JSPropertyKey>& key) {
    if (Testing::Is<JSString>(key)) {
        return ToIntegerIndex(key.CastTo<JSString>());
    } else {
        return -1;
    }
}

int64_t Conversion::ToLength(const Handle<JSNumber>& argument) {
    if (std::isnan(argument->Value())) {
        return 0x1FFFFFFFFFFFFFLL;
    }
    int64_t ret = ToIntegerValue(argument);
    if (ret <= 0)return 0;
    if (ret >= 0x1FFFFFFFFFFFFFLL) return 0x1FFFFFFFFFFFFFLL;
    return ret;
}

int64_t Conversion::ToLength(const Handle<JSValue>& argument) {
    return ToLength(ToNumber(argument));
}

Handle<JSNumber> Conversion::ScanInt(const Handle<JSString>& val, size_t& ptr, uint8_t base) {
    uint64_t result = 0;
    int16_t length = 0;
    int16_t overflow = 0;
    ScanDecimal(val, ptr, result, length, &overflow, base);
    if (length == 0) {
        return JSNumber::NaN();
    }

    if (overflow) {
        double value = static_cast<double>(result);
        if (overflow)
            value *= pow(static_cast<double>(base), static_cast<double>(overflow));
        return JSNumber::New(value);
    } else {
        return JSNumber::New(static_cast<int64_t>(result));
    }
}

Handle<JSNumber> Conversion::ScanFloat(const Handle<JSString>& val, size_t& ptr) {
    size_t len = val->Length();
    bool sign = false;

    switch (val->At(ptr)) {
        case '-':
            sign = true;
        case '+':
            ptr++;
            break;
    }

    if (ptr + 8 == len) {
        if (val->At(ptr) == 'I'&&
                val->At(ptr + 1) == 'n'&&
                val->At(ptr + 2) == 'f'&&
                val->At(ptr + 3) == 'i'&&
                val->At(ptr + 4) == 'n'&&
                val->At(ptr + 5) == 'i'&&
                val->At(ptr + 6) == 't'&&
                val->At(ptr + 7) == 'y') {
            return sign ? JSNumber::New(-std::numeric_limits<double>::infinity()) : JSNumber::Infinity();
        }
    }

    uint64_t s = 0;
    int16_t n = 0;
    int16_t k = 0;

    if (ptr == len) return JSNumber::NaN();
    char16_t ch = val->At(ptr);
    if (ch == '.') {
        ptr++;
        ScanDecimal(val, ptr, s, k, nullptr, 10);
        if (!k) {
            return JSNumber::NaN();
        }
    } else {
        ScanDecimal(val, ptr, s, k, &n, 10);
        n += k;
        if (ptr<len && val->At(ptr) == '.') {
            ptr++;
            ScanDecimal(val, ptr, s, k, nullptr, 10);
        }
    }
    /* Exponential part */
    if (ptr<len) {
        ch = val->At(ptr);
        if (ch == 'e' || ch == 'E') {
            size_t beforeExp = ptr;

            ptr++;
            bool sign = false;
            if (ptr < len) {
                ch = val->At(ptr);
                if (ch == '+') {
                    ptr++;
                } else if (ch == '-') {
                    sign = true;
                    ptr++;
                }
            }

            uint64_t expPart = 0;
            int16_t expPartLength = 0;
            ScanDecimal(val, ptr, expPart, expPartLength, nullptr, 10);
            if (expPart > 1000) expPart = 1000;

            if (!expPartLength) {
                ptr = beforeExp;
            }

            n += (sign ? -1 : 1)*static_cast<int16_t>(expPart);
        }
    }

    return JSNumber::New(AssembleDouble(s, n, static_cast<uint8_t>(k)));
}