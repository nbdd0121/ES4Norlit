#include "JSString.h"
#include "../gc/Heap.h"
#include "../util/HashMap.h"

#include "grammar/Scanner.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#include <typeinfo>
#include <algorithm>

using namespace norlit::js;
using namespace norlit::gc;
using namespace norlit::util;
using norlit::js::grammar::Scanner;

void JSString::ConvertUtf8ToUtf16(const char* from, char16_t* to) {
    size_t len = 0;
    for (size_t i = 0; from[i]; i++, len++) {
        unsigned char c = from[i];
        if (!(c & (1 << 7))) {
            to[len] = c;
        } else {
            if (!(c & (1 << 5))) {
                unsigned char n = from[++i];
                assert((n & 0xC0) == 0x80);
                to[len] = ((c & 31) << 6) | (n & 63);
            } else if (!(c & (1 << 4))) {
                unsigned char n = from[++i];
                unsigned char nn = from[++i];
                assert((n & 0xC0) == 0x80 && (nn & 0xC0) == 0x80);
                to[len] = ((c & 15) << 12) | ((n & 63) << 6) | (nn & 63);
            } else {
                unsigned char n = from[++i];
                unsigned char nn = from[++i];
                unsigned char nnn = from[++i];
                assert((n & 0xC0) == 0x80 && (nn & 0xC0) == 0x80 && (nnn & 0xC0) == 0x80);
                uint32_t codePoint = (((c & 7) << 18) | ((n & 63) << 12) | ((nn & 63) << 6) | (nnn & 63)) - 0x10000;
                to[len] = 0xD800 | (codePoint >> 10);
                to[++len] = 0xDC00 | (codePoint & 0x3FF);
            }
        }
    }
}

JSString* JSString::CreateASCIIShortString(size_t length, const char* str) {
    uintptr_t result = 0;
    /* Compact string to uintptr_t */
    for (size_t i = 0; i < length; i++) {
        result |= static_cast<uintptr_t>(static_cast<uint8_t>(str[i])) << (i * 8);
    }
    /* Reserve space for tag */
    result <<= 8;
    /* Add the tag to the result */
    result |= (length << 4) | 0 | 0x2;
    return reinterpret_cast<JSString*>(result);
}

JSString* JSString::CreateASCIIShortString(size_t length, const wchar_t* str) {
    uintptr_t result = 0;
    /* Compact string to uintptr_t */
    for (size_t i = 0; i < length; i++) {
        result |= static_cast<uintptr_t>(static_cast<uint8_t>(str[i])) << (i * 8);
    }
    /* Reserve space for tag */
    result <<= 8;
    /* Add the tag to the result */
    result |= (length << 4) | 0 | 0x2;
    return reinterpret_cast<JSString*>(result);
}

JSString* JSString::CreateUnicodeShortString(size_t length, const char* str) {
    char16_t buffer[MAX_UNICODE_SHORT_STRING_LENGTH];
    ConvertUtf8ToUtf16(str, buffer);

    uintptr_t result = 0;
    /* Compact string to uintptr_t */
    for (size_t i = 0; i < length; i++) {
        result |= static_cast<uintptr_t>(static_cast<uint16_t>(buffer[i])) << (i * 16);
    }
    /* Reserve space for tag */
    result <<= 16;
    /* Add the tag to the result */
    result |= (length << 4) | 0x8 | 0x2;
    return reinterpret_cast<JSString*>(result);
}

JSString* JSString::CreateUnicodeShortString(size_t length, const wchar_t* str) {
    uintptr_t result = 0;
    /* Compact string to uintptr_t */
    for (size_t i = 0; i < length; i++) {
        result |= static_cast<uintptr_t>(static_cast<uint16_t>(str[i])) << (i * 16);
    }
    /* Reserve space for tag */
    result <<= 16;
    /* Add the tag to the result */
    result |= (length << 4) | 0x8 | 0x2;
    return reinterpret_cast<JSString*>(result);
}

Handle<JSString> JSString::Intern(const Handle<JSString> str) {
    // Initialize on first use, better than pointer way
    static HashMap<JSString, JSString, true, true> internTable;

    Handle<JSString> ret = internTable.Get(str);
    if (ret) {
        return ret;
    } else {
        internTable.Put(str, str);
        return str;
    }
}

JSString::JSString(size_t length, const char* str) {
    // Make sure GC will not occur within constructor
    NoGC _;
    WriteBarrier(&string, ValueArray<char16_t>::New(length));
    char16_t* value = &string->At(0);
    ConvertUtf8ToUtf16(str, value);

    // Calculate hashcode at once
    uintptr_t hash = 0;
    for (size_t i = 0; i < length; i++) {
        hash = 31 * hash + value[i];
    }
    this->hash = hash;
}

JSString::JSString(size_t length, const wchar_t* str) {
    // Make sure GC will not occur within constructor
    NoGC _;
    WriteBarrier(&string, ValueArray<char16_t>::New(length));
    char16_t* array = &string->At(0);

    // Calculate hashcode at once
    uintptr_t hash = 0;
    for (size_t i = 0; i < length; i++) {
        char16_t ch = static_cast<char16_t>(str[i]);
        array[i] = ch;
        hash = 31 * hash + ch;
    }
    this->hash = hash;
}

void JSString::IterateField(const FieldIterator& iter) {
    iter(&string);
}

JSValue::Type JSString::VirtualGetType() const {
    return Type::kString;
}

Handle<JSString> JSString::New(const char* str) {
    // count utf8 code units of a utf8 string
    size_t length = 0;
    bool ascii = true;
    for (int i = 0; str[i]; i++, length++) {
        uint8_t c = str[i];
        if (c & (1 << 7)) {
            ascii = false;
            assert(c & (1 << 6));
            if (!(c & (1 << 5))) {
                i++;
            } else if (!(c & (1 << 4))) {
                i += 2;
            } else {
                assert(!(c & (1 << 3)));
                length++;
                i += 3;
            }
        }
    }
    if (ascii) {
        if (length <= MAX_ASCII_SHORT_STRING_LENGTH) {
            return CreateASCIIShortString(length, str);
        }
    } else {
        if (length <= MAX_UNICODE_SHORT_STRING_LENGTH) {
            return CreateUnicodeShortString(length, str);
        }
    }
    return Intern(new JSString(length, str));
}

Handle<JSString> JSString::New(const wchar_t* str) {
    size_t length = 0;
    bool ascii = true;
    for (; str[length]; length++) {
        if (static_cast<uint16_t>(str[length]) >= 0x80) {
            ascii = false;
        }
    }
    if (ascii) {
        if (length <= MAX_ASCII_SHORT_STRING_LENGTH) {
            return CreateASCIIShortString(length, str);
        }
    } else {
        if (length <= MAX_UNICODE_SHORT_STRING_LENGTH) {
            return CreateUnicodeShortString(length, str);
        }
    }
    return Intern(new JSString(length, str));
}

Handle<JSString> JSString::NewFromCString(const char* str) {
#ifdef _WIN32
    int bufSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    Handle<ValueArray<wchar_t>> buf = ValueArray<wchar_t>::New(bufSize);
    MultiByteToWideChar(CP_ACP, 0, str, -1, &buf->At(0), bufSize);
    // make sure buf is not moved
    NoGC _;
    return New(&buf->At(0));
#else
    return New(str);
#endif
}

bool JSString::Equals(const Handle<Object>& object) {
    /* Pointer comparision (shortcut) */
    if (this == object) {
        return true;
    }
    if (typeid(*object) != typeid(JSString)) {
        return false;
    }
    JSString* another = static_cast<JSString*>(object);
    size_t length = this->Length();
    if (another->Length() != length) {
        return false;
    }
    char16_t* thisContent = &this->string->At(0);
    char16_t* otherContent = &another->string->At(0);

    for (size_t i = 0; i < length; i++) {
        if (thisContent[i] != otherContent[i]) {
            return false;
        }
    }
    return true;
}

uintptr_t JSString::HashCode() {
    return hash;
}

Handle<ValueArray<char>> JSString::ToUTF8() {
    Handle<JSString> thisPtr = this;

    /* Count as UTF8 */
    int utf8Length = 0;
    for (size_t i = 0, len = thisPtr->Length(); i < len; i++, utf8Length++) {
        uint16_t c = thisPtr->At(i);
        if (c < 0x7F) {
        } else if (c < 0x7FF) {
            utf8Length++;
        } else if (c >= 0xD800 && c < 0xDC00) {
            uint16_t n = thisPtr->At(++i);
            assert(n >= 0xDC00 && c < 0xE000);
            utf8Length += 3;
        } else {
            utf8Length += 2;
        }
    }

    Handle<ValueArray<char>> result = ValueArray<char>::New(utf8Length + 1);

    for (size_t i = 0, len = 0; i < thisPtr->Length(); i++, len++) {
        uint16_t c = thisPtr->At(i);
        if (c < 0x7F) {
            result->At(len) = c & 0xFF;
        } else if (c < 0x7FF) {
            result->At(len) = (c >> 6) | 0xC0;
            result->At(++len) = (c & 0x3F) | 0x80;
        } else if (c >= 0xD800 && c < 0xDC00) {
            uint16_t n = thisPtr->At(++i);
            uint32_t codePoint = (((c - 0xD800) << 10) | (n - 0xDC00)) + 0x10000;
            result->At(len) = (codePoint >> 18) | 0xF0;
            result->At(++len) = ((codePoint >> 12) & 0x3F) | 0x80;
            result->At(++len) = ((codePoint >> 6) & 0x3F) | 0x80;
            result->At(++len) = (codePoint & 0x3F) | 0x80;
        } else {
            result->At(len) = (c >> 12) | 0xE0;
            result->At(++len) = ((c >> 6) & 0x3F) | 0x80;
            result->At(++len) = (c & 0x3F) | 0x80;
        }
    }

    result->At(utf8Length) = 0;
    return result;
}

Handle<ValueArray<char>> JSString::ToCString() {
#ifdef _WIN32
    Handle<ValueArray<wchar_t>> s = ToWChar();
    int bufSize = WideCharToMultiByte(CP_ACP, 0, &s->At(0), -1, NULL, 0, NULL, NULL);
    Handle<ValueArray<char>> buf = ValueArray<char>::New(bufSize + 1);
    WideCharToMultiByte(CP_ACP, 0, &s->At(0), -1, &buf->At(0), bufSize, NULL, NULL);
    return std::move(buf);
#else
    return ToUTF8();
#endif
}

Handle<ValueArray<wchar_t>> JSString::ToWChar() {
    Handle<JSString> thisPtr = this;
    size_t length = thisPtr->Length();
    Handle<ValueArray<wchar_t>> result = ValueArray<wchar_t>::New(length + 1);
    for (size_t i = 0; i < length; i++) {
        result->At(i) = thisPtr->At(i);
    }
    result->At(length) = 0;
    return result;
}

Handle<JSString> JSString::Substring(size_t start, size_t end) {
    std::wstring builder;
    for (size_t i = start; i < end; i++) {
        builder += At(i);
    }
    return New(builder.c_str());
}

Handle<JSString> JSString::Concat(const Handle<JSString>& left, const Handle<JSString>& right) {
    std::wstring builder;
    for (size_t i = 0, size = left->Length(); i < size; i++) {
        builder += left->At(i);
    }
    for (size_t i = 0, size = right->Length(); i < size; i++) {
        builder += right->At(i);
    }
    return New(builder.c_str());
}

Handle<JSString> JSString::Concat(std::initializer_list<Handle<JSString>> list) {
    std::wstring builder;
    for (const Handle<JSString>& h : list) {
        for (size_t i = 0, size = h->Length(); i < size; i++) {
            builder += h->At(i);
        }
    }
    return New(builder.c_str());
}

Handle<JSString> JSString::Trim() {
    std::wstring builder;
    size_t size = this->Length(), left, right;
    for (left = 0; left < size; left++) {
        char16_t ch = this->At(left);
        if (!Scanner::IsWhitespace(ch) && !Scanner::IsLineTerminator(ch)) {
            break;
        }
    }
    for (right = size - 1; right < size; right--) {
        char16_t ch = this->At(right);
        if (!Scanner::IsWhitespace(ch) && !Scanner::IsLineTerminator(ch)) {
            break;
        }
    }
    return this->Substring(left, right + 1);
}

Handle<JSString> JSString::TrimLeft() {
    std::wstring builder;
    size_t size = this->Length(), left;
    for (left = 0; left < size; left++) {
        char16_t ch = this->At(left);
        if (!Scanner::IsWhitespace(ch) && !Scanner::IsLineTerminator(ch)) {
            break;
        }
    }
    return this->Substring(left, size);
}