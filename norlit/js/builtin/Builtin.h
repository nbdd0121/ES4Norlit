#ifndef NORLIT_JS_BUILTIN_BUILTIN_H
#define NORLIT_JS_BUILTIN_BUILTIN_H

#include "../../gc/Array.h"
#include "../object/JSObject.h"

#define DEFINE_CTOR() \
	static gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);\
	static gc::Handle<object::JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<object::JSObject>&)
#define DEFINE_METHOD(name) static gc::Handle<JSValue> name(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&)
#define DEFINE_GETTER(name) static gc::Handle<JSValue> get_##name(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&)

namespace norlit {
namespace js {

class JSString;
class JSSymbol;

namespace builtin {

gc::Handle<JSValue> GetArg(const gc::Handle<gc::Array<JSValue>>& args, size_t index);
gc::Handle<gc::Array<JSValue>> GetRestArg(const gc::Handle<gc::Array<JSValue>>& args, size_t index);

gc::Handle<JSValue> eval(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
gc::Handle<JSValue> isFinite(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
gc::Handle<JSValue> isNaN(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
gc::Handle<JSValue> parseFloat(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
gc::Handle<JSValue> parseInt(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
gc::Handle<JSValue> decodeURI(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
gc::Handle<JSValue> decodeURIComponent(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
gc::Handle<JSValue> encodeURI(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
gc::Handle<JSValue> encodeURIComponent(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);

struct BuiltinObject {
    static gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<object::JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<object::JSObject>&);

    static gc::Handle<JSValue> assign(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> create(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> defineProperties(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> defineProperty(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> freeze(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> getOwnPropertyDescriptor(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> getOwnPropertyNames(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> getOwnPropertySymbols(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> getPrototypeOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> is(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> isExtensible(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> isFrozen(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> isSealed(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> keys(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> preventExtensions(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> seal(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> setPrototypeOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);

    struct prototype {
        static gc::Handle<JSValue> hasOwnProperty(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> isPrototypeOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> propertyIsEnumerable(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toLocaleString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> valueOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> get__proto__(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> set__proto__(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    };
};

struct Boolean {
    static gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<object::JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<object::JSObject>&);

    struct prototype {
        static gc::Handle<JSValue> toString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> valueOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    };
};

struct Symbol {
    static gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSString> SymbolDescriptiveString(const gc::Handle<JSSymbol>&);

    static gc::Handle<JSValue> for_(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> keyFor(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);

    struct prototype {
        static gc::Handle<JSValue> toString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> valueOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> Symbol_toPrimitive(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    };
};

#define ERROR_DEFINITION_GENERATE(className) struct className {\
static gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);\
static gc::Handle<object::JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<object::JSObject>&);\
struct prototype {\
	static gc::Handle<JSValue> toString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);\
};\
};

ERROR_DEFINITION_GENERATE(Error)
ERROR_DEFINITION_GENERATE(EvalError)
ERROR_DEFINITION_GENERATE(RangeError)
ERROR_DEFINITION_GENERATE(ReferenceError)
ERROR_DEFINITION_GENERATE(SyntaxError)
ERROR_DEFINITION_GENERATE(TypeError)
ERROR_DEFINITION_GENERATE(URIError)

#undef ERROR_DEFINITION_GENERATE

struct Number {
    static gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<object::JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<object::JSObject>&);

    static gc::Handle<JSValue> isFinite(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> isInteger(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> isNaN(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> isSafeInteger(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);

    struct prototype {
        static gc::Handle<JSValue> toExponential(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toFixed(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toLocaleString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toPrecision(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> valueOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    };
};

struct Math {
    static gc::Handle<JSValue> abs(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> acos(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> acosh(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> asin(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> asinh(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> atan(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> atanh(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> atan2(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> cbrt(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> ceil(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> clz32(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> cos(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> cosh(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> exp(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> expm1(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> floor(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> fround(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> hypot(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> imul(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> log(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> log1p(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> log10(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> log2(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> max(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> min(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> pow(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> random(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> round(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> sign(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> sin(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> sinh(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> sqrt(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> tan(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> tanh(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> trunc(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
};

struct Date {
    static gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<object::JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<object::JSObject>&);

    static gc::Handle<JSValue> now(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> parse(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> UTC(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);

    struct prototype {
        static gc::Handle<JSValue> getDate(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getDay(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getFullYear(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getHours(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getMilliseconds(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getMinutes(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getMonth(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getSeconds(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getTime(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getTimezoneOffset(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getUTCDate(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getUTCDay(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getUTCFullYear(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getUTCHours(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getUTCMilliseconds(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getUTCMinutes(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getUTCMonth(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getUTCSeconds(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> getYear(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setDate(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setFullYear(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setHours(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setMilliseconds(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setMinutes(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setMonth(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setSeconds(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setTime(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setUTCDate(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setUTCFullYear(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setUTCHours(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setUTCMilliseconds(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setUTCMinutes(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setUTCMonth(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setUTCSeconds(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> setYear(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toDateString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toGMTString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toISOString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toJSON(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toLocaleDateString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toLocaleTimeString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toTimeString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toUTCString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> valueOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> Symbol_toPrimitive(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    };
};

struct String {
    static gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<object::JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<object::JSObject>&);

    static gc::Handle<JSValue> fromCharCode(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> fromCodePoint(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<JSValue> raw(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);

    struct prototype {
        static gc::Handle<JSValue> charAt(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> charCodeAt(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> codePointAt(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> concat(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> endsWith(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> includes(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> indexOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> lastIndexOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> localCompare(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> match(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> normalize(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> repeat(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> replace(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> search(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> splice(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> split(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> startsWith(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> substring(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toLocaleLowerCase(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toLocaleUpperCase(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toLowerCase(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toString(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> toUpeerCase(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> trim(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> valueOf(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> Symbol_iterator(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);

        static gc::Handle<JSValue> substr(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> anchor(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> big(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> blink(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> bold(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> fixed(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> fontcolor(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> fontsize(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> italics(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> link(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> small(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> strike(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> sub(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
        static gc::Handle<JSValue> sup(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    };
};

struct RegExp {
    static gc::Handle<JSValue> Call(const gc::Handle<JSValue>&, const gc::Handle<gc::Array<JSValue>>&);
    static gc::Handle<object::JSObject> Construct(const gc::Handle<gc::Array<JSValue>>&, const gc::Handle<object::JSObject>&);

    DEFINE_GETTER(Symbol_species);

    struct prototype {
        DEFINE_METHOD(exec);
        DEFINE_GETTER(flags);
        DEFINE_GETTER(global);
        DEFINE_GETTER(ignoreCase);
        DEFINE_METHOD(Symbol_match);
        DEFINE_GETTER(multiline);
        DEFINE_METHOD(Symbol_replace);
        DEFINE_METHOD(Symbol_search);
        DEFINE_GETTER(source);
        DEFINE_METHOD(Symbol_split);
        DEFINE_GETTER(sticky);
        DEFINE_METHOD(test);
        DEFINE_METHOD(toString);
        DEFINE_GETTER(unicode);
    };
};

struct BuiltinArray {
    DEFINE_CTOR();

    DEFINE_METHOD(from);
    DEFINE_METHOD(isArray);
    DEFINE_METHOD(of);
    DEFINE_GETTER(Symbol_species);

    struct prototype {
        DEFINE_METHOD(concat);
        DEFINE_METHOD(copyWithin);
        DEFINE_METHOD(entries);
        DEFINE_METHOD(every);
        DEFINE_METHOD(fill);
        DEFINE_METHOD(filter);
        DEFINE_METHOD(find);
        DEFINE_METHOD(findIndex);
        DEFINE_METHOD(forEach);
        DEFINE_METHOD(indexOf);
        DEFINE_METHOD(join);
        DEFINE_METHOD(keys);
        DEFINE_METHOD(lastIndexOf);
        DEFINE_METHOD(map);
        DEFINE_METHOD(pop);
        DEFINE_METHOD(push);
        DEFINE_METHOD(reduce);
        DEFINE_METHOD(reduceRight);
        DEFINE_METHOD(reverse);
        DEFINE_METHOD(shift);
        DEFINE_METHOD(slice);
        DEFINE_METHOD(some);
        DEFINE_METHOD(sort);
        DEFINE_METHOD(splice);
        DEFINE_METHOD(toLocaleString);
        DEFINE_METHOD(toString);
        DEFINE_METHOD(unshift);
        DEFINE_METHOD(values);
    };

    DEFINE_METHOD(Iterator_next);
};

struct Generator {
    struct prototype {
        DEFINE_METHOD(next);
        DEFINE_METHOD(return_);
        DEFINE_METHOD(throw_);
    };
};

}
}
}

#undef DEFINE_METHOD
#undef DEFINE_GETTER

#endif

