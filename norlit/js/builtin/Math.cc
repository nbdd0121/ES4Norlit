#include "Builtin.h"

#include "../JSNumber.h"
#include "../Conversion.h"
#include "../Objects.h"
#include "../Exception.h"

#include "../object/Exotics.h"
#include "../vm/Realm.h"

#include <cmath>
#include <random>

using namespace norlit::gc;
using namespace norlit::js;
using namespace norlit::js::vm;
using namespace norlit::js::object;
using namespace norlit::js::builtin;

#define STD_MATH_WRAPPER(name) Handle<JSValue> Math::name(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {\
	double x = Conversion::ToNumberValue(GetArg(args, 0));\
	return JSNumber::New(::name(x));\
}

STD_MATH_WRAPPER(abs);
STD_MATH_WRAPPER(acos);
STD_MATH_WRAPPER(acosh);
STD_MATH_WRAPPER(asin);
STD_MATH_WRAPPER(asinh);
STD_MATH_WRAPPER(atan);
STD_MATH_WRAPPER(atanh);

Handle<JSValue> Math::atan2(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    double y = Conversion::ToNumberValue(GetArg(args, 0));
    double x = Conversion::ToNumberValue(GetArg(args, 1));
    return JSNumber::New(std::atan2(y, x));
}

STD_MATH_WRAPPER(cbrt);
STD_MATH_WRAPPER(ceil);

Handle<JSValue> Math::clz32(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    uint32_t n = Conversion::ToUInt32(Conversion::ToNumber(GetArg(args, 0)));
    int32_t p = 32;
    while (n) {
        p--;
        n >>= 1;
    }
    return JSNumber::New(p);
}

STD_MATH_WRAPPER(cos);
STD_MATH_WRAPPER(cosh);
STD_MATH_WRAPPER(exp);
STD_MATH_WRAPPER(expm1);
STD_MATH_WRAPPER(floor);

Handle<JSValue> Math::fround(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    double x = Conversion::ToNumberValue(GetArg(args, 0));
    x = static_cast<double>(static_cast<float>(x));
    return JSNumber::New(x);
}

Handle<JSValue> Math::hypot(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    double ret = 0;
    for (size_t i = 0, size = args->Length(); i < size; i++) {
        double item = Conversion::ToNumberValue(args->Get(i));
        ret += item * item;
    }
    return JSNumber::New(std::sqrt(ret));
}

Handle<JSValue> Math::imul(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    uint32_t a = Conversion::ToUInt32(Conversion::ToNumber(GetArg(args, 0)));
    uint32_t b = Conversion::ToUInt32(Conversion::ToNumber(GetArg(args, 0)));
    int32_t product = a * b;
    return JSNumber::New(product);
}

STD_MATH_WRAPPER(log);
STD_MATH_WRAPPER(log1p);
STD_MATH_WRAPPER(log10);
STD_MATH_WRAPPER(log2);

Handle<JSValue> Math::max(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    double ret = -std::numeric_limits<double>::infinity();
    for (size_t i = 0, size = args->Length(); i < size; i++) {
        double item = Conversion::ToNumberValue(args->Get(i));
        if (std::isnan(item)) {
            return JSNumber::NaN();
        }
        if (item > ret) {
            ret = item;
        } else if (item == 0 && ret == 0) {
            // item is +0 or ret is +0
            if (!std::signbit(item) || !std::signbit(ret)) {
                ret = 0;
            }
        }
    }
    return JSNumber::New(ret);
}

Handle<JSValue> Math::min(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    double ret = std::numeric_limits<double>::infinity();
    for (size_t i = 0, size = args->Length(); i < size; i++) {
        double item = Conversion::ToNumberValue(args->Get(i));
        if (std::isnan(item)) {
            return JSNumber::NaN();
        }
        if (item < ret) {
            ret = item;
        } else if (item == 0 && ret == 0) {
            // item is -0 or ret is -0
            if (std::signbit(item) || !std::signbit(ret)) {
                ret = -0.0;
            }
        }
    }
    return JSNumber::New(ret);
}

Handle<JSValue> Math::pow(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    double x = Conversion::ToNumberValue(GetArg(args, 0));
    double y = Conversion::ToNumberValue(GetArg(args, 1));
    return JSNumber::New(std::pow(x, y));
}

Handle<JSValue> Math::random(const Handle<JSValue>&, const Handle<Array<JSValue>>&) {
    static std::default_random_engine engine{ std::random_device()() };
    double rand = std::generate_canonical<double, std::numeric_limits<double>::digits>(engine);
    return JSNumber::New(rand);
}

STD_MATH_WRAPPER(round);

Handle<JSValue> Math::sign(const Handle<JSValue>&, const Handle<Array<JSValue>>& args) {
    double x = Conversion::ToNumberValue(GetArg(args, 0));
    if (x > 0) {
        return JSNumber::One();
    } else if (x < 0) {
        return JSNumber::New(-1);
    } else {
        return JSNumber::New(x);
    }
}

STD_MATH_WRAPPER(sin);
STD_MATH_WRAPPER(sinh);
STD_MATH_WRAPPER(sqrt);
STD_MATH_WRAPPER(tan);
STD_MATH_WRAPPER(tanh);
STD_MATH_WRAPPER(trunc);
