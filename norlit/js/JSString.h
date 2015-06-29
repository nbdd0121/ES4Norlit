#ifndef NORLIT_JS_JSSTRING_H
#define NORLIT_JS_JSSTRING_H

#include "JSPropertyKey.h"
#include "../gc/Array.h"

#include <cstddef>
#include <stdexcept>
#include <initializer_list>

namespace norlit {
namespace js {

class JSString final : public JSPropertyKey {
    /*
    * Tagged String Representation
    *
    * Byte 0: llllu010
    *         ^   ^
    *     length  unicode flag
    *
    * If unicode flag is true, Byte 2-3(32bit), 2-7(64bit) stores 1 or 3 utf16 characters
    * If unicode flag is false, Byte 1 to Byte 3/7 stores 3 or 7 ascii characters
    *
    */
  private:
    static const size_t MAX_ASCII_SHORT_STRING_LENGTH = sizeof(void*) - 1;
    static const size_t MAX_UNICODE_SHORT_STRING_LENGTH = sizeof(void*) / 2 - 1;

    static void ConvertUtf8ToUtf16(const char* from, char16_t* to);
    static JSString* CreateASCIIShortString(size_t length, const char* str);
    static JSString* CreateASCIIShortString(size_t length, const wchar_t* str);
    static JSString* CreateUnicodeShortString(size_t length, const char* str);
    static JSString* CreateUnicodeShortString(size_t length, const wchar_t* str);
    static gc::Handle<JSString> Intern(const gc::Handle<JSString>);

    gc::ValueArray<char16_t>* string = nullptr;
    // Cache hashcode
    uintptr_t hash;

    JSString(size_t, const char*);
    JSString(size_t, const wchar_t*);

    bool IsShortStringUnicode() const;
    size_t GetShortStringLength() const;
    char16_t GetShortStringChar(size_t) const;

    virtual bool Equals(const gc::Handle<gc::Object>&) override final;
    virtual void IterateField(const gc::FieldIterator& iter) override final;
    virtual Type VirtualGetType() const override final;
    virtual uintptr_t HashCode() override final;
  public:
    static gc::Handle<JSString> New(const char*);
    static gc::Handle<JSString> New(const wchar_t*);
    static gc::Handle<JSString> NewFromCString(const char*);

    static gc::Handle<JSString> Concat(const gc::Handle<JSString>&, const gc::Handle<JSString>&);
    static gc::Handle<JSString> Concat(std::initializer_list<gc::Handle<JSString>>);

    size_t Length() const;
    char16_t At(size_t) const;

    gc::Handle<gc::ValueArray<char>> ToUTF8();
    gc::Handle<gc::ValueArray<char>> ToCString();
    gc::Handle<gc::ValueArray<wchar_t>> ToWChar();
    gc::Handle<JSString> Substring(size_t start, size_t end);
    gc::Handle<JSString> Trim();
    gc::Handle<JSString> TrimLeft();
};

inline bool JSString::IsShortStringUnicode() const {
    return (reinterpret_cast<uintptr_t>(this) & 8) == 8;
}

inline size_t JSString::GetShortStringLength() const {
    uintptr_t bits = reinterpret_cast<uintptr_t>(this);
    return (bits >> 4) & 0xF;
}

inline char16_t JSString::GetShortStringChar(size_t pos) const {
    uintptr_t bits = reinterpret_cast<uintptr_t>(this);
    if (IsShortStringUnicode()) {
        return (bits >> (pos + 1) * 16) & 0xFFFF;
    } else {
        return (bits >> (pos + 1) * 8) & 0xFF;
    }
}

inline size_t JSString::Length() const {
    if (IsShortString()) {
        return GetShortStringLength();
    } else {
        return string->Length();
    }
}

inline char16_t JSString::At(size_t pos) const {
    if (pos > Length()) {
        throw std::range_error{"Index out of bound"};
    }
    if (IsShortString()) {
        return GetShortStringChar(pos);
    } else {
        return string->At(pos);
    }
}

}
}

#endif
