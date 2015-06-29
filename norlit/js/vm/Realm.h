#ifndef NORLIT_JS_VM_REALM_H
#define NORLIT_JS_VM_REALM_H

#include "../JSValue.h"
#include "../object/JSObject.h"
#include "Environment.h"

namespace norlit {
namespace js {
namespace vm {

class Realm : public gc::Object {
    object::JSObject* Array_ = nullptr;
    object::JSObject* ArrayBuffer_ = nullptr;
    object::JSObject* ArrayBufferPrototype_ = nullptr;
    object::JSObject* ArrayIteratorPrototype_ = nullptr;
    object::JSObject* ArrayPrototype_ = nullptr;
    object::JSObject* ArrayProto_values_ = nullptr;
    object::JSObject* Boolean_ = nullptr;
    object::JSObject* BooleanPrototype_ = nullptr;
    object::JSObject* DataView_ = nullptr;
    object::JSObject* DataViewPrototype_ = nullptr;
    object::JSObject* Date_ = nullptr;
    object::JSObject* DatePrototype_ = nullptr;
    object::JSObject* decodeURI_ = nullptr;
    object::JSObject* decodeURIComponent_ = nullptr;
    object::JSObject* encodeURI_ = nullptr;
    object::JSObject* encodeURIComponent_ = nullptr;
    object::JSObject* Error_ = nullptr;
    object::JSObject* ErrorPrototype_ = nullptr;
    object::JSObject* escape_ = nullptr;
    object::JSObject* eval_ = nullptr;
    object::JSObject* EvalError_ = nullptr;
    object::JSObject* EvalErrorPrototype_ = nullptr;
    object::JSObject* Float32Array_ = nullptr;
    object::JSObject* Float32ArrayPrototype_ = nullptr;
    object::JSObject* Float64Array_ = nullptr;
    object::JSObject* Float64ArrayPrototype_ = nullptr;
    object::JSObject* Function_ = nullptr;
    object::JSObject* FunctionPrototype_ = nullptr;
    object::JSObject* Generator_ = nullptr;
    object::JSObject* GeneratorFunction_ = nullptr;
    object::JSObject* GeneratorPrototype_ = nullptr;
    object::JSObject* Int8Array_ = nullptr;
    object::JSObject* Int8ArrayPrototype_ = nullptr;
    object::JSObject* Int16Array_ = nullptr;
    object::JSObject* Int16ArrayPrototype_ = nullptr;
    object::JSObject* Int32Array_ = nullptr;
    object::JSObject* Int32ArrayPrototype_ = nullptr;
    object::JSObject* isFinite_ = nullptr;
    object::JSObject* isNaN_ = nullptr;
    object::JSObject* IteratorPrototype_ = nullptr;
    object::JSObject* JSON_ = nullptr;
    object::JSObject* Map_ = nullptr;
    object::JSObject* MapIteratorPrototype_ = nullptr;
    object::JSObject* MapPrototype_ = nullptr;
    object::JSObject* Math_ = nullptr;
    object::JSObject* Number_ = nullptr;
    object::JSObject* NumberPrototype_ = nullptr;
    object::JSObject* Object_ = nullptr;
    object::JSObject* ObjectPrototype_ = nullptr;
    object::JSObject* ObjProto_toString_ = nullptr;
    object::JSObject* parseFloat_ = nullptr;
    object::JSObject* parseInt_ = nullptr;
    object::JSObject* Promise_ = nullptr;
    object::JSObject* PromisePrototype_ = nullptr;
    object::JSObject* Proxy_ = nullptr;
    object::JSObject* RangeError_ = nullptr;
    object::JSObject* RangeErrorPrototype_ = nullptr;
    object::JSObject* ReferenceError_ = nullptr;
    object::JSObject* ReferenceErrorPrototype_ = nullptr;
    object::JSObject* Reflect_ = nullptr;
    object::JSObject* RegExp_ = nullptr;
    object::JSObject* RegExpPrototype_ = nullptr;
    object::JSObject* Set_ = nullptr;
    object::JSObject* SetIteratorPrototype_ = nullptr;
    object::JSObject* SetPrototype_ = nullptr;
    object::JSObject* String_ = nullptr;
    object::JSObject* StringIteratorPrototype_ = nullptr;
    object::JSObject* StringPrototype_ = nullptr;
    object::JSObject* Symbol_ = nullptr;
    object::JSObject* SymbolPrototype_ = nullptr;
    object::JSObject* SyntaxError_ = nullptr;
    object::JSObject* SyntaxErrorPrototype_ = nullptr;
    object::JSObject* ThrowTypeError_ = nullptr;
    object::JSObject* TypedArray_ = nullptr;
    object::JSObject* TypedArrayPrototype_ = nullptr;
    object::JSObject* TypeError_ = nullptr;
    object::JSObject* TypeErrorPrototype_ = nullptr;
    object::JSObject* Uint8Array_ = nullptr;
    object::JSObject* Uint8ArrayPrototype_ = nullptr;
    object::JSObject* Uint8ClampedArray_ = nullptr;
    object::JSObject* Uint8ClampedArrayPrototype_ = nullptr;
    object::JSObject* Uint16Array_ = nullptr;
    object::JSObject* Uint16ArrayPrototype_ = nullptr;
    object::JSObject* Uint32Array_ = nullptr;
    object::JSObject* Uint32ArrayPrototype_ = nullptr;
    object::JSObject* unescape_ = nullptr;
    object::JSObject* URIError_ = nullptr;
    object::JSObject* URIErrorPrototype_ = nullptr;
    object::JSObject* WeakMap_ = nullptr;
    object::JSObject* WeakMapPrototype_ = nullptr;
    object::JSObject* WeakSet_ = nullptr;
    object::JSObject* WeakSetPrototype_ = nullptr;


    object::JSObject *globalThis_ = nullptr;

    GlobalEnvironment *globalEnv_ = nullptr;

    virtual void IterateField(const gc::FieldIterator&) override final;

  public:
    using Intrinsic = gc::Handle<object::JSObject> (Realm::*)();

    Realm();

    gc::Handle<object::JSObject> GetArray() {
        return Array_;
    }
    gc::Handle<object::JSObject> ArrayBuffer() {
        return ArrayBuffer_;
    }
    gc::Handle<object::JSObject> ArrayBufferPrototype() {
        return ArrayBufferPrototype_;
    }
    gc::Handle<object::JSObject> ArrayIteratorPrototype() {
        return ArrayIteratorPrototype_;
    }
    gc::Handle<object::JSObject> ArrayPrototype() {
        return ArrayPrototype_;
    }
    gc::Handle<object::JSObject> ArrayProto_values() {
        return ArrayProto_values_;
    }
    gc::Handle<object::JSObject> Boolean() {
        return Boolean_;
    }
    gc::Handle<object::JSObject> BooleanPrototype() {
        return BooleanPrototype_;
    }
    gc::Handle<object::JSObject> DataView() {
        return DataView_;
    }
    gc::Handle<object::JSObject> DataViewPrototype() {
        return DataViewPrototype_;
    }
    gc::Handle<object::JSObject> Date() {
        return Date_;
    }
    gc::Handle<object::JSObject> DatePrototype() {
        return DatePrototype_;
    }
    gc::Handle<object::JSObject> decodeURI() {
        return decodeURI_;
    }
    gc::Handle<object::JSObject> decodeURIComponent() {
        return decodeURIComponent_;
    }
    gc::Handle<object::JSObject> encodeURI() {
        return encodeURI_;
    }
    gc::Handle<object::JSObject> encodeURIComponent() {
        return encodeURIComponent_;
    }
    gc::Handle<object::JSObject> Error() {
        return Error_;
    }
    gc::Handle<object::JSObject> ErrorPrototype() {
        return ErrorPrototype_;
    }
    gc::Handle<object::JSObject> eval() {
        return eval_;
    }
    gc::Handle<object::JSObject> EvalError() {
        return EvalError_;
    }
    gc::Handle<object::JSObject> EvalErrorPrototype() {
        return EvalErrorPrototype_;
    }
    gc::Handle<object::JSObject> Float32Array() {
        return Float32Array_;
    }
    gc::Handle<object::JSObject> Float32ArrayPrototype() {
        return Float32ArrayPrototype_;
    }
    gc::Handle<object::JSObject> Float64Array() {
        return Float64Array_;
    }
    gc::Handle<object::JSObject> Float64ArrayPrototype() {
        return Float64ArrayPrototype_;
    }
    gc::Handle<object::JSObject> Function() {
        return Function_;
    }
    gc::Handle<object::JSObject> FunctionPrototype() {
        return FunctionPrototype_;
    }
    gc::Handle<object::JSObject> Generator() {
        return Generator_;
    }
    gc::Handle<object::JSObject> GeneratorFunction() {
        return GeneratorFunction_;
    }
    gc::Handle<object::JSObject> GeneratorPrototype() {
        return GeneratorPrototype_;
    }
    gc::Handle<object::JSObject> Int8Array() {
        return Int8Array_;
    }
    gc::Handle<object::JSObject> Int8ArrayPrototype() {
        return Int8ArrayPrototype_;
    }
    gc::Handle<object::JSObject> Int16Array() {
        return Int16Array_;
    }
    gc::Handle<object::JSObject> Int16ArrayPrototype() {
        return Int16ArrayPrototype_;
    }
    gc::Handle<object::JSObject> Int32Array() {
        return Int32Array_;
    }
    gc::Handle<object::JSObject> Int32ArrayPrototype() {
        return Int32ArrayPrototype_;
    }
    gc::Handle<object::JSObject> isFinite() {
        return isFinite_;
    }
    gc::Handle<object::JSObject> isNaN() {
        return isNaN_;
    }
    gc::Handle<object::JSObject> IteratorPrototype() {
        return IteratorPrototype_;
    }
    gc::Handle<object::JSObject> JSON() {
        return JSON_;
    }
    gc::Handle<object::JSObject> Map() {
        return Map_;
    }
    gc::Handle<object::JSObject> MapIteratorPrototype() {
        return MapIteratorPrototype_;
    }
    gc::Handle<object::JSObject> MapPrototype() {
        return MapPrototype_;
    }
    gc::Handle<object::JSObject> Math() {
        return Math_;
    }
    gc::Handle<object::JSObject> Number() {
        return Number_;
    }
    gc::Handle<object::JSObject> NumberPrototype() {
        return NumberPrototype_;
    }
    gc::Handle<object::JSObject> Object() {
        return Object_;
    }
    gc::Handle<object::JSObject> ObjectPrototype() {
        return ObjectPrototype_;
    }
    gc::Handle<object::JSObject> ObjProto_toString() {
        return ObjProto_toString_;
    }
    gc::Handle<object::JSObject> parseFloat() {
        return parseFloat_;
    }
    gc::Handle<object::JSObject> parseInt() {
        return parseInt_;
    }
    gc::Handle<object::JSObject> Promise() {
        return Promise_;
    }
    gc::Handle<object::JSObject> PromisePrototype() {
        return PromisePrototype_;
    }
    gc::Handle<object::JSObject> Proxy() {
        return Proxy_;
    }
    gc::Handle<object::JSObject> RangeError() {
        return RangeError_;
    }
    gc::Handle<object::JSObject> RangeErrorPrototype() {
        return RangeErrorPrototype_;
    }
    gc::Handle<object::JSObject> ReferenceError() {
        return ReferenceError_;
    }
    gc::Handle<object::JSObject> ReferenceErrorPrototype() {
        return ReferenceErrorPrototype_;
    }
    gc::Handle<object::JSObject> Reflect() {
        return Reflect_;
    }
    gc::Handle<object::JSObject> RegExp() {
        return RegExp_;
    }
    gc::Handle<object::JSObject> RegExpPrototype() {
        return RegExpPrototype_;
    }
    gc::Handle<object::JSObject> Set() {
        return Set_;
    }
    gc::Handle<object::JSObject> SetIteratorPrototype() {
        return SetIteratorPrototype_;
    }
    gc::Handle<object::JSObject> SetPrototype() {
        return SetPrototype_;
    }
    gc::Handle<object::JSObject> String() {
        return String_;
    }
    gc::Handle<object::JSObject> StringIteratorPrototype() {
        return StringIteratorPrototype_;
    }
    gc::Handle<object::JSObject> StringPrototype() {
        return StringPrototype_;
    }
    gc::Handle<object::JSObject> Symbol() {
        return Symbol_;
    }
    gc::Handle<object::JSObject> SymbolPrototype() {
        return SymbolPrototype_;
    }
    gc::Handle<object::JSObject> SyntaxError() {
        return SyntaxError_;
    }
    gc::Handle<object::JSObject> SyntaxErrorPrototype() {
        return SyntaxErrorPrototype_;
    }
    gc::Handle<object::JSObject> ThrowTypeError() {
        return ThrowTypeError_;
    }
    gc::Handle<object::JSObject> TypedArray() {
        return TypedArray_;
    }
    gc::Handle<object::JSObject> TypedArrayPrototype() {
        return TypedArrayPrototype_;
    }
    gc::Handle<object::JSObject> TypeError() {
        return TypeError_;
    }
    gc::Handle<object::JSObject> TypeErrorPrototype() {
        return TypeErrorPrototype_;
    }
    gc::Handle<object::JSObject> Uint8Array() {
        return Uint8Array_;
    }
    gc::Handle<object::JSObject> Uint8ArrayPrototype() {
        return Uint8ArrayPrototype_;
    }
    gc::Handle<object::JSObject> Uint8ClampedArray() {
        return Uint8ClampedArray_;
    }
    gc::Handle<object::JSObject> Uint8ClampedArrayPrototype() {
        return Uint8ClampedArrayPrototype_;
    }
    gc::Handle<object::JSObject> Uint16Array() {
        return Uint16Array_;
    }
    gc::Handle<object::JSObject> Uint16ArrayPrototype() {
        return Uint16ArrayPrototype_;
    }
    gc::Handle<object::JSObject> Uint32Array() {
        return Uint32Array_;
    }
    gc::Handle<object::JSObject> Uint32ArrayPrototype() {
        return Uint32ArrayPrototype_;
    }
    gc::Handle<object::JSObject> URIError() {
        return URIError_;
    }
    gc::Handle<object::JSObject> URIErrorPrototype() {
        return URIErrorPrototype_;
    }
    gc::Handle<object::JSObject> WeakMap() {
        return WeakMap_;
    }
    gc::Handle<object::JSObject> WeakMapPrototype() {
        return WeakMapPrototype_;
    }
    gc::Handle<object::JSObject> WeakSet() {
        return WeakSet_;
    }
    gc::Handle<object::JSObject> WeakSetPrototype() {
        return WeakSetPrototype_;
    }

    gc::Handle<object::JSObject> GetGlobalObject() {
        return globalThis_;
    }

    gc::Handle<GlobalEnvironment> GetGlobalEnvironment() {
        return globalEnv_;
    }
};

}
}
}

#endif